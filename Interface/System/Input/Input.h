#pragma once
#include <iostream>

enum class InputState { Pressed, Released, Repeated };

struct RawInputEvent {
    //KeyCode code;
    InputState state;
    double timestamp; // 极其重要，解决帧率不匹配
};

class IInputSystem {
public:
    virtual ~IInputSystem() = default;

    // 1. 依然保留轮询（用于某些特殊逻辑，如每帧移动）
    virtual float GetAxis(const std::string& actionName) = 0;
    virtual bool GetAction(const std::string& actionName) = 0;

    // 2. 引入事件回调（现代架构核心）
    //using ActionCallback = std::function<void(InputState state)>;
    virtual void BindAction(const std::string& actionName, ActionCallback callback) = 0;

    // 3. 驱动层调用的更新接口
    virtual void PushRawEvent(const RawInputEvent& event) = 0;
    virtual void Tick(float deltaTime) = 0; // 处理缓冲区和连按逻辑
};