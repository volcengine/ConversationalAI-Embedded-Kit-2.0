<h1 align="center"><img src="https://iam.volccdn.com/obj/volcengine-public/pic/volcengine-icon.png"></h1>
<h1 align="center">ConversationalAI Embedded Kit 2.0 </h1>

## 快速开始

## 环境配置
- 可运行环境：Linux
- 音频采集和播放无法选择具体设备，建议测试时不要外接设备，如：屏幕、耳机等
- 示例程序不具备3A能力，因此可能会存在误识别的情况，属于正常现象。可以通过空格按键控制音频的输入，避免误识别 
- 本示例仅支持pulseaudio音频服务，因此需要先安装pulseaudio
```shell
//  安装依赖
apt-get install pulseaudio libpulse-dev

// 启动音频服务
pulseaudio --exit-idle-time=-1 --daemon
```

## 基本用法
```shell
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
在examples/conversation_ai/examples/high_quality_solution/macos/configs/conv_ai_config.json文件中，需要修改以下配置项：
```shell
{
  "mode": 0,  // 0: rtc, 1: websocket
  "bot_id": "************",  // 智能体ID，通过控制台获取
  "ver": 1,
  "iot": {
    "instance_id": "************",  // 实例ID，通过控制台获取
    "product_key": "************",  // 产品Key，通过控制台获取
    "product_secret": "************",  // 产品Secret，通过控制台获取
    "device_name": "************"  // 设备名称，自定义
  },
  "rtc": {
    "log_level": 3,    // rtc 日志等级，1：info，2：warn，3：error
    "audio" : {
      "publish": true,
      "subscribe": true,
      "codec": 3
    },
    "video" : {
      "publish": false,
      "subscribe": false,
      "codec": 1
    },
    "params": [
      "{\"audio\":{\"codec\":{\"internal\":{\"enable\":1}}}}"
    ]
  }
}
```
## 编译
该example为 Linux 平台编译
```shell
mkdir build
cd build
cmake -DENABLE_RTC_MODE=ON .. // 通过ENABLE_RTC_MODE开启RTC通道
make
make install
```

## 运行
```shell
./bin/volc_conv_ai_demo

空格 按键：停止或者开始音频采集（起始运行时为停止状态）
```
