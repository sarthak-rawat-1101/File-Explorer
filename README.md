```
# File Explorer
* Linux based File Explorer built using C++
* Entire code is written in main.cpp
* To run:
```
```sh
g++ main.cpp
./a.out
```
#### Assumptions

| Symbol | Meaning |
| ------ | ------ |
| ```~``` | The folder where the application was started |
| ```.``` | Current Directory |

## Functionalities
---
#### _Normal Mode_
Normal mode is the default mode of the application. Can be used to explore the current directory and navigate the filesystem. 
| Keys | Function |
| ------ | ------ |
| ``` UP_ARROW``` | Traverse in upward direction |
| ```DOWN_ARROW``` | Traverse in downward direction |
| ```LEFT_ARROW``` | Move back to the previously visited directory |
| ```RIGHT_ARROW``` | Move forward to previously visited directory |
| ```BACKSPACE``` | Move one directory back |
| ```l``` | Scroll down when cursor reaches to bottom |
| ```k``` | Scroll up when cursor reaches is at top |
| ```ENTER``` | Opens a file or directory |
| ```h``` | Takes back to the home folder (The folder where the application was started) |
| ```q``` | Application terminates |
---
#### _Command Mode_
The application enters the Command button whenever ```:```  (colon) key is pressed. In the
command mode, the user will be able to enter different commands. All commands appear in the
status bar at the bottom.

| Command | Syntax/ Function |
| ------ | ------ |
| Copy | ```copy <source_file(s)> <destination_directory>```|
| Move | ```move <source_file(s)> <destination_directory>```|
| Rename | ```rename <old_filename> <new_filename>```|
|Create File | ```create_file <file_name> <destination_path>```|
|Create Directory | ```create_dir <dir_name> <destination_path>```|
|Delete File  | ```delete_file <file_path>```|
|Delete Directory | ```delete_dir <dir_path>```|
|Goto | ```goto <location>```|
|Search | ```search <file_name>``` or ```search <directory_name>```|
| ```ESC``` | The application goes back to Normal Mode |

### NOTE 
- In order to quit the application, if in ```Normal Mode``` press ```q``` else if in ```Command Mode``` first return to ```Normal Mode```  by pressing ```ESC``` and then simply press ```q```.

```
