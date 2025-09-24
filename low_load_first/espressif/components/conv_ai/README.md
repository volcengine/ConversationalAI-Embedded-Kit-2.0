# Volc Conversation AI Kit

Volc Conversation AI Kit 是一个端到端的智能硬件对话开发方案，兼容主流 IoT 芯片，为智能家居、穿戴设备等产品提供低延迟、高自然的语音交互能力，让您的硬件会听、会看、会说话。


# 集成方法
```
// step 1: 初始化引擎
volc_event_handler_t volc_event_handler = {
    .on_volc_event = ...,
    .on_volc_conversation_status = ...,
    .on_volc_audio_data = ...,
    .on_volc_video_data = ...,
    .on_volc_message_data = ...
};
error = volc_create(...);

// step 2: 启动会话
...
error = volc_start(...);

// step 3: 等待建联成功
while (!is_ready) {
    sleep(1);
    printf("waiting for volc realtime to be ready...\n");
}

// step 4: 发送音频数据
while(true) {
    volc_send_audio_data(...);
    ...
}

// step 5: 停止会话
volc_stop(...);

// step 6: 销毁引擎
volc_destroy(...);
```
