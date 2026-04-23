
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "cJSON.h"

#include "util/volc_ringbuf.h"

#include "volc_conv_ai.h"

#define AUDIO_CHANNEL_NUM 1
#define AUDIO_FRAME_PER_SECOND (32000)
#define AUDIO_FRAME_MS (1000 / (AUDIO_FRAME_PER_SECOND / AUDIO_FRAME_LEN))

#define WS_BUFFER_CLEAR "{\"type\":\"input_audio_buffer.clear\"}"
#define WS_SESSION_UPDATE "{\"event_id\":\"event_OgjwihjHg\",\"type\":\"session.update\",\"session\":{\"object\":\"realtime.session\",\"model\":\"\",\"config\":{\"ASRConfig\":{\"TurnDetectionMode\":0}},\"agent_config\":{\"WelcomeMessage\":\"这是一个覆盖智能体上的欢迎语.\"}}}"

typedef struct {
	uint8_t* audio_rec_buf;
    bool commit;
    char* bot_id;
    char* video_rec_buf;
    int video_frame_len;
    int mode;
    int frame_len;
    int sample_rate;
    int audio_frame_ms;
    volc_ringbuf_t ring_buf;
	pa_simple* p_capture;
	pa_simple* p_playback;
    pa_sample_spec format;
    pthread_t audio_playback_task;
    volc_engine_t engine;
} realtime_ws_demo_t;

static volatile sig_atomic_t running = false;
static volatile sig_atomic_t interrupt = false;
static volatile sig_atomic_t start = false;
static volatile sig_atomic_t stop = false;
static volatile sig_atomic_t destory = false;
static volatile sig_atomic_t clear = false;
static volatile sig_atomic_t video_upload = false;
static volatile sig_atomic_t exit_request = false;
static volatile sig_atomic_t session_update = false;
static struct termios original_term;
static int tick = 0;

static void __get_fps(void) {
    static uint64_t last_sec = 0;
    static int fps = 0;
    struct timespec now_time;
    fps++;
    clock_gettime(CLOCK_REALTIME, &now_time);
    if (now_time.tv_sec != last_sec) {
        last_sec = now_time.tv_sec;
        printf("send data fps: %d\n", fps);
        fps = 0;
    }
}

static uint64_t __get_time_ms(void) {
    struct timespec now_time;
    clock_gettime(CLOCK_REALTIME, &now_time);
    return now_time.tv_sec * 1000 + now_time.tv_nsec / 1000000;
}

// 恢复终端设置
static void __restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

// 信号处理函数
static void __handle_signal(int sig) {
    if (sig == SIGIO) {
        char buf[1];
        int n = read(STDIN_FILENO, buf, sizeof(buf));
        if (n > 0) {
            switch (buf[0]) {
                case ' ':
                    running = !running;
                    printf("\n状态: %s\n", running ? "运行中" : "已停止");
                    break;
                case 'i':
                    printf("\n状态: 打断\n");
                    interrupt = true;
                    break;
                case 'o':
                    printf("\n状态: stop\n");
                    stop = true;
                    break;
                case 'a':
                    printf("\n状态: start\n");
                    start = true;
                    break;
                case 'd':
                    printf("\n状态: destory\n");
                    destory = true;
                    break;
                case 'c':
                    printf("\n状态: 清除\n");
                    clear = true;
                    break;
                case 'v':
                    printf("\n状态: 视频上传\n");
                    video_upload = true;
                    break;
                case 'u':
                    printf("\n状态: session update\n");
                    session_update = true;
                    break;
                default:
                    break;
            }
        }
    } else if (sig == SIGINT) {
        exit_request = true;
    }
}

// 设置异步I/O
static void __setup_async_io(void) {
    struct sigaction sa;
    int flags;

    // 保存原始终端设置
    tcgetattr(STDIN_FILENO, &original_term);
    
    // 设置退出时恢复终端
    atexit(__restore_terminal);
    
    // 配置信号处理
    sa.sa_handler = __handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGIO, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // 设置标准输入为异步模式
    flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK);
    
    // 设置当前进程为接收SIGIO信号的进程
    fcntl(STDIN_FILENO, F_SETOWN, getpid());
    
    // 修改终端设置：禁用行缓冲和回显
    struct termios term = original_term;
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

static char* __load_config_from_file(const char* filename) {
    FILE* config_fp = fopen(filename, "rb");
    if (config_fp == NULL) {
        printf("failed to open %s for reading.\n", filename);
        return NULL;
    }
    fseek(config_fp, 0, SEEK_END);
    int config_len = ftell(config_fp);
    if (config_len < 0) {
        printf("Failed to seek to end of %s.\n", filename);
        fclose(config_fp);
        return NULL;
    }
    fseek(config_fp, 0, SEEK_SET);
    char* config_data = (char*) malloc(config_len + 1);
    if (config_data == NULL) {
        printf("Malloc config data fail\n");
        fclose(config_fp);
        return NULL;
    }
    memset(config_data, 0, config_len + 1);
    size_t read_size = fread(config_data, 1, config_len, config_fp);
    if (read_size != config_len) {
        printf("Failed to read %s, expected %d bytes, got %zu bytes.\n", filename, config_len, read_size);
        free(config_data);
        fclose(config_fp);
        return NULL;
    }
    fclose(config_fp);
	return config_data;
}

static int __load_video_file(realtime_ws_demo_t* demo) {
    FILE* video_fp = fopen("send_video.h264", "rb");
    if (video_fp == NULL) {
        printf("failed to open send_video.h264 for reading.\n");
        return -1;
    }
    fseek(video_fp, 0, SEEK_END);
    demo->video_frame_len = ftell(video_fp);
    if (demo->video_frame_len < 0) {
        printf("Failed to seek to end of send_video.h264.\n");
        fclose(video_fp);
        return -1;
    }
    fseek(video_fp, 0, SEEK_SET);
    demo->video_rec_buf = (char*) realloc(demo->video_rec_buf, demo->video_frame_len + 1);
    if (demo->video_rec_buf == NULL) {
        printf("Realloc video_rec_buf failed\n");
        fclose(video_fp);
        return -1;
    }
    memset(demo->video_rec_buf, 0, demo->video_frame_len + 1);
    size_t read_size = fread(demo->video_rec_buf, 1, demo->video_frame_len, video_fp);
    if (read_size != demo->video_frame_len) {
        printf("Failed to read send_video.h264, expected %d bytes, got %zu bytes.\n", demo->video_frame_len, read_size);
        free(demo->video_rec_buf);
        fclose(video_fp);
        return -1;
    }
    fclose(video_fp);
    return 0;
}

static void* __audio_playback_task(void* arg) {
    int len = 0;
    int error  = 0;
    uint8_t *buffer = NULL;
    realtime_ws_demo_t* demo = (realtime_ws_demo_t*)arg;
    buffer = (uint8_t *)malloc(demo->frame_len);
    if (NULL == buffer) {
        printf("malloc buffer failed\n");
        return NULL;
    }
    while (!exit_request) {
            len = volc_ringbuf_read(demo->ring_buf,(char *)buffer, sizeof(buffer));
            if (len > 0) {
                pa_simple_write(demo->p_playback, buffer, len, &error);
                if (error != 0) {
                    printf("pa_simple_write error: %d\n", error);
                }
            } else {
                usleep(100 * 1000);
            }
    }
    free(buffer);
    return NULL;
}

static bool is_ready = false;
static void _on_volc_event(volc_engine_t handle, volc_event_t* event, void* user_data)
{
    switch (event->code) {
        case VOLC_EV_CONNECTED:
            is_ready = true;
            printf("Volc Engine connected\n");
            break;
        case VOLC_EV_DISCONNECTED:
            is_ready = false;
            printf("Volc Engine disconnected\n");
            break;
        default:
            printf("Volc Engine event: %d\n", event->code);
            break;
    }
}

static void _on_volc_conversation_status(volc_engine_t handle, volc_conv_status_e status, void* user_data)
{
    printf("conversation status changed: %d\n", status);
    if (status == VOLC_CONV_STATUS_THINKING) {
    }
}

static void _on_volc_audio_data(volc_engine_t handle, const void* data_ptr, size_t data_len, volc_audio_frame_info_t* info_ptr, void* user_data)
{
	int error = 0;
	realtime_ws_demo_t* demo = (realtime_ws_demo_t*)user_data;
	if (demo == NULL) {
		printf("demo is NULL\n");
		return;
	}
    if (volc_ringbuf_write(demo->ring_buf, (char *)data_ptr, data_len) != data_len) {
        printf("write audio data to ring buf fail!!!!!!!!\n");
    }
    static FILE* fp = NULL;
    if (fp == NULL) {
        fp = fopen("audio_playback.pcm", "wb");
        if (fp == NULL) {
            printf("Failed to open audio_data.pcm for writing.\n");
            return;
        }
    }
    if (data_ptr != NULL && data_len > 0) {
        fwrite(data_ptr, 1, data_len, fp);
        fflush(fp);
    } else {
        printf("Received empty audio data.\n");
    }
}

static void _on_volc_video_data(volc_engine_t handle, const void* data_ptr, size_t data_len, volc_video_frame_info_t* info_ptr, void* user_data)
{
}

static void __on_subtitle_message_received(cJSON* root) {
    if (NULL == root) {
        return;
    }
    cJSON * type_obj = cJSON_GetObjectItem(root, "type");
    if (type_obj != NULL && strcmp("subtitle", cJSON_GetStringValue(type_obj)) == 0) {
        cJSON* data_obj_arr = cJSON_GetObjectItem(root, "data");
        cJSON* obji = NULL;
        cJSON_ArrayForEach(obji, data_obj_arr) {
            cJSON* user_id_obj = cJSON_GetObjectItem(obji, "userId");
            cJSON* text_obj = cJSON_GetObjectItem(obji, "text");
            if (user_id_obj && text_obj) {
                printf("subtitle:%s:%s\n", cJSON_GetStringValue(user_id_obj), cJSON_GetStringValue(text_obj));
            }
        }
    }
}

static void __send_conversation_item_create(realtime_ws_demo_t* demo, const char* call_id) {
    volc_message_info_t msg_info = { 0 };
    cJSON* root = cJSON_CreateObject();
    cJSON* item = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "type", cJSON_CreateString("conversation.item.create"));
    cJSON_AddItemToObject(root, "item", item);
    cJSON_AddItemToObject(item, "call_id", cJSON_CreateString(call_id));
    cJSON_AddItemToObject(item, "type", cJSON_CreateString("function_call_output"));
    cJSON_AddItemToObject(item, "object", cJSON_CreateString("realtime.item"));
    cJSON_AddItemToObject(item, "output", cJSON_CreateString("上海天气25摄氏度"));

    char* json_str = cJSON_Print(root);
    if (json_str == NULL) {
        printf("cJSON_Print failed\n");
        return;
    }
    printf("json_str:%s\n", json_str);
    cJSON_Delete(root);
    if (json_str) {
        msg_info.is_binary = true;
        volc_send_message(demo->engine, json_str, strlen(json_str), &msg_info);
        free(json_str);
    }
}

static void __handle_ws_conversation_item_created_call(realtime_ws_demo_t* demo, cJSON* root) {
    cJSON* item_obj = NULL;
    cJSON* call_id_obj = NULL;
    cJSON* type_obj = NULL;
    cJSON* name_obj = NULL;

    item_obj = cJSON_GetObjectItem(root, "item");
    if (item_obj == NULL) {
        printf("item_obj is NULL\n");
        return;
    }
    call_id_obj = cJSON_GetObjectItem(item_obj, "call_id");
    if (call_id_obj == NULL) {
        printf("call_id_obj is NULL\n");
        return;
    }
    type_obj = cJSON_GetObjectItem(item_obj, "type");
    if (type_obj == NULL) {
        printf("type_obj is NULL\n");
        return;
    }
    if (strcmp("function_call", cJSON_GetStringValue(type_obj)) == 0) {
        name_obj = cJSON_GetObjectItem(item_obj, "name");
        if (name_obj == NULL) {
            printf("name_obj is NULL\n");
            return;
        }
        // 处理function_call
        // 处理get_current_weather fc
        if (strcmp("天气", cJSON_GetStringValue(name_obj)) == 0) {
            __send_conversation_item_create(demo, cJSON_GetStringValue(call_id_obj));
        } else {
            printf("unknown function_call name:%s\n", cJSON_GetStringValue(name_obj));
        }
    }
}

static void __handle_ws_message(realtime_ws_demo_t* demo, const void* message, size_t size) {
    cJSON* root = NULL;
    cJSON* type_obj = NULL;
    root = cJSON_Parse((const char*)message);
    if (NULL == root) {
        printf("parse json buffer failed\n");
        return;
    }
    type_obj = cJSON_GetObjectItem(root, "type");
    if (type_obj != NULL && strcmp("conversation.item.created", cJSON_GetStringValue(type_obj)) == 0) {
        __handle_ws_conversation_item_created_call(demo, root);
    } else {
        printf("%s\n", (char*)message);
    }

    if (root) {
        cJSON_Delete(root);
    }
}

static void __handle_rtc_message(realtime_ws_demo_t* demo, const void* message, size_t size) {
    cJSON* root = NULL;
    if (((uint8_t*)message)[0] == 's' && ((uint8_t*)message)[1] == 'u' && ((uint8_t*)message)[2] == 'b' && ((uint8_t*)message)[3] == 'v') {
        // root = cJSON_Parse(((uint8_t *)message) + 8);
        root = cJSON_Parse((const char *)(((uint8_t *)message) + 8));
        __on_subtitle_message_received(root);
    } else {
        printf("message size: %d data: %s\n", (int)size, (char*) message);
    }
    if (root) {
        cJSON_Delete(root);
    }
}

static void _on_volc_message_data(volc_engine_t handle, const void* message, size_t size, volc_message_info_t* info_ptr, void* user_data)
{
    
    realtime_ws_demo_t* demo = (realtime_ws_demo_t*)user_data;
    static int cnt = 0;
    printf("--------------%d----------------\r\n", cnt++);
    switch(demo->mode) {
        case VOLC_MODE_RTC:
            __handle_rtc_message(demo, message, size);
            break;
        case VOLC_MODE_WS:
            __handle_ws_message(demo, message, size);
            break;
        default:
            break;
    }
    printf("------------------------------\r\n");
}

static int __parse_demo_config(const char* config, realtime_ws_demo_t* handle) {
    cJSON* root = NULL;
    cJSON* obj_item = NULL;
    if (NULL == config) {
        printf("invalid input args\n");
        return -1;
    }
    root = cJSON_Parse(config);
    if (NULL == root) {
        printf("parse json buffer failed\n");
        return -1;
    }
    obj_item = cJSON_GetObjectItem(root, "bot_id");
    if (NULL == obj_item) {
        printf("parse bot_id failed\n");
        goto err_out_label;
    }
    handle->bot_id = strdup(cJSON_GetStringValue(obj_item));

    obj_item = cJSON_GetObjectItem(root, "mode");
    if (NULL == obj_item) {
        printf("parse mode failed\n");
        goto err_out_label;
    }
    handle->mode = (int)cJSON_GetNumberValue(obj_item);
    if (handle->mode == 0) {
        // rtc
        handle->frame_len = 320;
        handle->sample_rate = 8000;
    } else {
        handle->frame_len = 3200;
        handle->sample_rate = 16000;
    }
    handle->audio_frame_ms = (1000 / (AUDIO_FRAME_PER_SECOND / handle->frame_len));

    printf("get bot id from config file: %s, mode: %d\n", handle->bot_id, handle->mode);
    cJSON_Delete(root);
    return 0;
err_out_label:
    cJSON_Delete(root);
    return -1;
}

static int _build_ws_message(const char* msg, uint8_t** out_buf, size_t* out_len) {
    size_t msg_len = strlen(msg) + 1;
    *out_len = msg_len - 1;
    *out_buf = (uint8_t*)malloc(msg_len);
    if (!*out_buf) {
        printf("malloc failed\n");
        return -1;
    }
    memcpy(*out_buf, msg, msg_len);
    (*out_buf)[msg_len - 1] = 0;
    return 0;
}

static int _ws_clear_buffer(realtime_ws_demo_t* demo) {
    uint8_t* clear = NULL;
    size_t clear_len = 0;
    volc_message_info_t msg_info = {0};
    int ret = _build_ws_message(WS_BUFFER_CLEAR, &clear, &clear_len);
    if (ret != 0) {
        printf("build clear message failed");
        return ret;
    }
    ret = volc_send_message(demo->engine, clear, clear_len, NULL);
    if (clear) {
        free(clear);
    }
    return ret;
}

int main(int argc, const char* argv[]){
	char* config_data = NULL;
    int error = 0;
    uint64_t last_time_ms = 0;
    uint64_t diff_time_ms = 0;
	realtime_ws_demo_t demo = {0};

	if ((config_data = __load_config_from_file("conv_ai_config.json")) == NULL) {
		printf("load config from file fail\n");
		return -1;
	}

    // if (__load_video_file(&demo) != 0) {
    //     printf("load video file failed\n");
    //     return -1;
    // }

    if (__parse_demo_config(config_data, &demo) != 0) {
        printf("parse demo config failed");
        return -1;
    }

    demo.commit = false;
	demo.audio_rec_buf = (uint8_t*)malloc(demo.frame_len);
	if (demo.audio_rec_buf == NULL) {
		printf("malloc audio rec buf fail\n");
		return -1;
	}
    demo.ring_buf = volc_ringbuf_create(demo.frame_len * 1000);
    if (demo.ring_buf == NULL) {
        printf("create ring buf fail\n");
        goto err_out_label;
    }

    demo.format.format = PA_SAMPLE_S16NE;
    demo.format.rate = demo.sample_rate;
    demo.format.channels = AUDIO_CHANNEL_NUM;
    demo.p_capture = pa_simple_new(NULL, "Capture", PA_STREAM_RECORD, NULL, "Capture", &demo.format, NULL, NULL, &error);
    if (NULL == demo.p_capture) {
        printf("capture pa_simple_new failed: %s\n", pa_strerror(error));
        goto err_out_label;
    }
    demo.p_playback = pa_simple_new(NULL, "Playback", PA_STREAM_PLAYBACK, NULL, "Playback", &demo.format, NULL, NULL, &error);
    if (NULL == demo.p_playback) {
        printf("playback pa_simple_new failed: %s\n", pa_strerror(error));
        goto err_out_label;
    }

    volc_event_handler_t volc_event_handler = {.on_volc_event = _on_volc_event,
                                            .on_volc_conversation_status = _on_volc_conversation_status,
                                            .on_volc_audio_data = _on_volc_audio_data,
                                            .on_volc_video_data = _on_volc_video_data,
                                            .on_volc_message_data = _on_volc_message_data};
    printf("demo.engine : %p\n",demo.engine);
    printf("volc_create event_handler 为NULL");
    error = volc_create(&demo.engine, config_data, &volc_event_handler, &demo);

    free(config_data);
    if (error != 0) {
        printf("volc_create failed: %d\n", error);
        goto err_out_label;
    }
    printf("demo.engine : %p\n",demo.engine);
    volc_opt_t opt = {0};
    opt.mode = demo.mode;

    // // create后直接destroy
    // printf("创建后直接销毁\n");
    // volc_destroy(demo.engine);
    // printf("销毁成功...\n");
    // return 0;

    // 验证create后直接destroy 程序不崩溃
    // printf("创建后直接销毁\n");
    // volc_destroy(demo.engine);
    // printf("销毁成功...\n");
    // return 0;


    // 验证引擎为NULL 调用start、stop、destroy 程序不崩溃
    // volc_start(demo.engine, &opt);
    // demo.engine = NULL 
    // volc_start(NULL, &opt);
    // volc_stop(NULL);
    // volc_destroy(NULL);
    // return 0;

    // 验证重复多次调用start
    // printf("第一次调用start....");
    // volc_start(demo.engine,&opt);
    // printf("第二次调用start....");
    // volc_start(demo.engine,&opt);
    // printf("第三次调用start....");
    // volc_start(demo.engine,&opt);
    // return 0;


    opt.bot_id = demo.bot_id;
    volc_start(demo.engine, &opt);

    while (!is_ready) {
        sleep(1);
        printf("waiting for volc realtime to be ready...\n");
    }
    printf("volc realtime is ready.\n");


    __setup_async_io();
    printf("键盘监听程序已启动\n");
    printf("按空格键开始/停止, 按i键打断, 按c键清除, 按v键开启视频上传, 按s键stop/start, 按d键 按Ctrl+C退出\n");
    printf("当前状态: 已停止\n");
    pthread_create(&demo.audio_playback_task, NULL, __audio_playback_task, &demo);

    volc_audio_frame_info_t info = {0};
    info.data_type = VOLC_AUDIO_DATA_TYPE_PCM;
    volc_video_frame_info_t video_info = {0};
    while (!exit_request) {
        // printf("volc....................\n");
        if (running) {
            last_time_ms = __get_time_ms();
            // __get_fps();
            if (pa_simple_read(demo.p_capture, demo.audio_rec_buf, demo.frame_len, &error) < 0) {
                fprintf(stderr, "录音失败: %s\n", pa_strerror(error));
                break;
            }
            info.commit = false;
            volc_send_audio_data(demo.engine, demo.audio_rec_buf, demo.frame_len, &info);
            diff_time_ms = __get_time_ms() - last_time_ms;
            if (diff_time_ms < demo.audio_frame_ms) {
                usleep(demo.audio_frame_ms - diff_time_ms);
            }
            demo.commit = true;
        } else {
            if (demo.commit) {
                info.commit = true;
                volc_send_audio_data(demo.engine, demo.audio_rec_buf, demo.frame_len, &info);
                demo.commit = false;
                printf("stop send audio data\n");
            }
            usleep(100000);
        }
        if (video_upload) {
            video_info.data_type = VOLC_VIDEO_DATA_TYPE_H264;
            if ((tick++ % 5) == 0) {
                volc_send_video_data(demo.engine, demo.video_rec_buf, demo.video_frame_len, &video_info);
            }
        }
        if (interrupt) {
            interrupt = false;
            volc_interrupt(demo.engine);
        }
        if (clear) {
            clear = false;
            _ws_clear_buffer(&demo);
        }
        if (stop) {
            stop = false;
            volc_stop(demo.engine);
        }
        if (start) {
            start = false;
            volc_start(demo.engine,&opt);
        }
        if (destory) {
            destory = false;
            volc_destroy(demo.engine);
            demo.engine = NULL; // 防止后续代码意外使用已释放的句柄
        }
        if (session_update) {
            session_update = false;
            volc_update(demo.engine, WS_SESSION_UPDATE, strlen(WS_SESSION_UPDATE));
        }
    }
    pthread_join(demo.audio_playback_task, NULL);

err_out_label:
	if (demo.audio_rec_buf) {
		free(demo.audio_rec_buf);
	}
	if (demo.p_capture) {
		pa_simple_free(demo.p_capture);
	}
	if (demo.p_playback) {
		pa_simple_drain(demo.p_playback, &error);
		pa_simple_free(demo.p_playback);
	}
	getchar();
	return 0;
}