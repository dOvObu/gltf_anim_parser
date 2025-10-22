# gltf_anim_parser

Привет 👋

В этом репозитории находится парсер и вьювер gltf анимаций написанный на c++
Он зависит от этих библиотек:
- glad
- glew
- nlohmann/json
- ReneNyffenegger/cpp-base64

Пользоваться так 👇

```
#include "WindowsFactory.h"
#include "GLFW.h"

int main()
{
  WindowsFactory wf;
  wf.Init();
  Window w = wf.CreateWindow("hi", 600,400);
  GLTF model;
  model.LoadFromHDDToRAM("some_model.gltf");
  float time = 0.f;
  while (w.IsOpened())
  {
    float time = glfwGetTime();
    float clipLength = model.GetClipLength(0);
    while(time > clipLength) time -= clipLength;
    model.SampleClip(0, time);
    w.Swap();
  }
}
```
