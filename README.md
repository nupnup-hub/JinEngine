# JinEngine

Game engine for studying graphics and software architecture 
  
![EngineImage00](https://github.com/nupnup-hub/JinEngine/assets/59456231/18817b30-8ff2-4e56-8001-730b64ed297a)
(Unreal engine4 sun temple model) 

![EngineImage01](https://github.com/nupnup-hub/JinEngine/assets/59456231/b46dd7fd-1a05-41ed-b6e1-cb9c2d6812f1)
(Amazon Lumberyard Bistro model)

## Features
### Graphis
- Dynamic global illumination
  - Restir gi
  - SVGF denoiser
- Physical based local illumination
- Ambient occlusion
  - SSAO
  - HBAO+
- Shadow
  - hard, soft shadow map
- Image processing
  - Bloom
  - Tone mapping
- Anti alising
  - Fxaa
- Culling
  - Hardware culling using graphics API
  - HZB
  - Frustum culling with scene accelerator Structure(cpu)
  - Clustered light culling

### Engine
- Manage engine resource(Load, store, import)
- Manage program memory
- Edit scene object


## Platform
|  |Directx12|...|
|------|---|---|
|Window| :heavy_check_mark: | |
|...| | | 

## Guide
- [Overview](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/Overview.md)
- [Documentation](https://github.com/nupnup-hub/JinEngine/tree/Main/docs)

## Third party
- [DirectX TK](https://github.com/microsoft/DirectXTK)
- [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)
- [ImGui](https://github.com/ocornut/imgui)
- [stb](https://github.com/nothings/stb/blob/master/LICENSE)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [LibTIFF](https://github.com/libsdl-org/libtiff)
- 
  
  


