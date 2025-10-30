<h1 align="center"><img src="https://iam.volccdn.com/obj/volcengine-public/pic/volcengine-icon.png"></h1>
<h1 align="center">ConversationalAI Embedded Kit 2.0</h1>

## 介绍
硬件对话智能体，是一个端到端的智能硬件对话开发平台，兼容主流 IoT 芯片，可快速帮助开发者将低延迟、高自然的 AI 对话能力集成到智能硬件中，让智能硬件会听、会看、会说话，适用于 AI 玩具、智能穿戴设备(AI眼镜，智能手表等设备)、陪伴机器人、智能家居、教育硬件、具身智能设备等场景。

### 功能特性
- **AI 实时语音对话**: 与智能体进行自然流畅的实时语音对话，如同与真人交流，支持随时插话打断。
- **语音识别**: 将用户语音实时转写为文本，供大模型分析理解、生成字幕等。
- **大模型处理**: 解析输入文本，并生成语义响应，驱动智能体对话逻辑。
- **语音合成**: 将大模型生成的文字回复转化为语音。
- **降噪**: 结合音频 3A 技术和 AI 降噪算法，能够兼顾强降噪与高保真，确保在嘈杂的环境中有效去除背景噪音，保留清晰的人声。
- **打断智能体**: 在对话过程中，用户可以随时打断智能体的语音输出，实现双向互动。
- **视频互动**: 接入视觉理解模型，使智能体能够理解实时视频画面或指定外部图片，从而实现感知环境、理解真人行为、图像问答等视觉交互。
- **Function calling**: 允许大模型识别用户对话中的特定需求，并在内容的过程中调用外部函数实现天气查询、数学计算等功能。如处理实时数据检索、文件处理、数据库查询等，从而扩展智能体的服务能力和应用场景。
- **实时字幕**: 实时将用户和智能体的对话内容转化为文字，可用于字幕渲染或存储。

## 快速体验

### 步骤一：前置准备
参考[快速入门](https://www.volcengine.com/docs/6348/1806625)开通服务并搭建硬件对话智能体。

### 步骤二：运行设备端
请根据你使用的硬件开发板，选择对应的设备端部署教程：
#### 低负载方案
- 乐鑫 ESP32-S3-Korvo-2: [运行设备端_乐鑫](examples/low_load_solution/espressif/README.md)
- MacOS方案: [运行设备端_MacOS](examples/low_load_solution/macos_platform/README.md)
#### 高性能方案
- 乐鑫 ESP32-S3-Korvo-2: [运行设备端_乐鑫](examples/high_quality_solution/espressif/README.md)
- MacOS方案: [运行设备端_MacOS](examples/high_quality_solution/macos_platform/README.md)

## 技术交流
欢迎加入我们的技术交流群或提出Issue，一起探讨技术，一起学习进步。
<div align=center><img src="resource/image/tech_support.png" width="200"></div>