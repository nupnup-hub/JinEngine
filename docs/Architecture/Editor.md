# Editor
  - Component dependency

![EditorDependency](https://github.com/nupnup-hub/JinEngine/assets/59456231/7a0dd5ec-59f0-41ce-ad82-c5c50bd6d7cf)

# Detail
  - Manage engine gui feature
  - The display scenario consist of control (JEditorPage, JEditorWindow) and implementation(JGui)
  - Can transition(do, undo)
  - The event will be updated one frame later for integrity of the resource
  - Class
      - JEditorManager
        - Manage all page
        - Control editor update flow
      - JEditorPage
        - Manage window
        - Control window state
      - JEditorWindow
        - Configure the Gui display scenario control according to window characteristics
      - JGui 
        - Connect the caller to the implementation according to Api
        - Control gui state
        - Manage displayed gui data(click, intersect, ...)
 
# Specific task senario
 - Display object explorer gui

![DisplayObjectExp drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/d315d800-f3a1-40a1-b479-a2c42cf7b62d)

 - Editor event process

![EditorEventProcess drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/283363fe-0210-4efc-9254-80054d250148)


   
   
