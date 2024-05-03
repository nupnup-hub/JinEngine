# Type Reflection
 
## Detail
- Define unique data per type
    - Identification(name, guid)
    - Method and parameter information
    - Instance data
    - Memory allocater
    - Initialize function
    - Lazy destruction
- Can access type unique data with interface pointer easily
- Can access registered parameter or method callable by name
- Consists list structure(child own parent pointer)
- Use static structure construction func for register

## Class overview
- JReflectionInfo 
    - Manage type per event(called by main loop)
- JTypeInfo
    - Own type per data 
- JMethodInfo
    - Own method callable 
    - Own method parameter and return type hint
- JParameterInfo
    - Own parameter get set callable 
    - Own parameter return type hint

## Diagram
![TypeReflection](https://github.com/nupnup-hub/JinEngine/assets/59456231/95e3a76b-8046-4952-982f-30926246b0ba)

