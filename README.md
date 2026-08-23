# gltf_anim_parser

Привет 👋

В этом репозитории находится парсер и вьювер gltf анимаций написанный на c++

Он зависит от этих библиотек:
- glad
- glew
- glm
- stb_image
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

<a href="http://www.wtfpl.net/"><img
       src="http://www.wtfpl.net/wp-content/uploads/2012/12/wtfpl-badge-4.png"
       width="80" height="15" alt="WTFPL" /></a>
