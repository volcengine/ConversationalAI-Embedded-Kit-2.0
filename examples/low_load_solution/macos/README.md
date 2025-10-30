<h1 align="center"><img src="https://iam.volccdn.com/obj/volcengine-public/pic/volcengine-icon.png"></h1>
<h1 align="center">ConversationalAI Embedded Kit 2.0 </h1>

## 快速开始

## 基本用法
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
    volc_send_video_data(...);
}
// step 5: 停止会话
volc_stop(...);
// step 6: 销毁引擎
volc_destroy(...);
```

## 配置文件说明
```
{
  "mode": 1,                            // 0: rtc, 1: websocket 
  "bot_id": "botT***0XL",               // 智能体ID，通过控制台获取
  "ver": 1,
  "iot": {
    "instance_id": "68998***c082",      // 实例ID，通过控制台获取
    "product_key": "68999***787c",      // 产品KEY，通过控制台获取
    "product_secret": "f45***985",      // 产品秘钥，通过控制台获取
    "device_name": "hu***v5",           // 设备名，可自行指定
    "host": "http://***.bytedance.net"  // 物理网平台域名，通过控制台获取
  },
  "ws": {
    "aigw_path": "/v1/realtime"         // 网关域名，通过控制台获取
  },
  "rtc": {
    "log_level": 3,                     // rtc 日志等级，1：info，2：warn，3：error
    "audio": {
      "publish": true,                  // 是否发布音频
      "subscribe": true,                // 是否订阅音频
      "codec": 4                        // 音频编码格式，1：OPUS，2：G722，3：AACLC，4：G711A，5：G711U
    },
    "video": {
      "publish": true,                  // 是否发布视频
      "subscribe": true,                // 是否订阅视频
      "codec": 1                        // 视频编码格式，1：H264
    },
    "params": [                       // rtc 自定义参数，可选，格式为 json 字符串数组
      "{\"audio\":{\"codec\":{\"internal\":{\"enable\":1}}}}"
    ]
  }
}
```
## 编译
该example为 MacOS 平台编译
```
mkdir build
cd build
cmake -DENABLE_RTC_MODE=ON .. // 通过ENABLE_RTC_MODE开启RTC通道
make
make install
```
## 运行
请确保当前目录下包含配置文件 `conv_ai_config.json`，并修改为正确的配置信息。
```
./bin/volc_conv_ai_demo
```
