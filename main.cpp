#include <bits/stdc++.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include<sys/wait.h> 
#include <sys/types.h>
#include <fcntl.h>

#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif
#define KEY_ARROW_CHAR1 2791
#define KEY_ESC 27
#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_LEFT 68
#define KEY_RIGHT 67
#define KEY_ENTER 10
#define KEY_BACK 127

using namespace std;

struct file_meta{
    string name;
    off_t  size;
    mode_t mode;
    time_t mtime;
};

struct termios old_terminal;
struct termios new_terminal;

int ws_row = 42;
int ws_col = 42;
int crr_line = 1;
int dir_window_top = 1;
int dir_window_bot = 0;
int dir_window_left  = 3;
int dir_window_right = 3;
string crr_dir = ".";
vector<file_meta> files;
vector<string> timeline;
int tl_len = 0;
int tl_crr = 0;

void init_timeline(){
    timeline.push_back(crr_dir);
    tl_crr = 0;
    tl_len = 1;
}

void insert_into_timeline(string path){
    if(tl_crr == timeline.size() - 1){
        timeline.push_back(path);
        tl_crr++;
        tl_len++;
    }else{
        timeline[++tl_crr] = path;
        tl_len = tl_crr + 1;
    }
}

void enter_raw_mode(){
    tcgetattr(STDIN_FILENO, &old_terminal);
    new_terminal = old_terminal;
    new_terminal.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_terminal);//changes will occur after flush
}
void exit_raw_mode(){
    tcsetattr(STDIN_FILENO,TCSANOW,&old_terminal);//changes will occur now
}

void move_cursor(int x,int y) {
	cout << "\033[" << x << ";" << y << "H";
}
void clear_line(){
    cout << "\033[K";
}

void clear_screen(){
    printf("%c[2J", KEY_ESC);
}

void display_arrow(){
    move_cursor(crr_line, 1);
    cout << ">";
}

void display_line(string line){
    for(int i = dir_window_left; i < line.length() and i <= dir_window_right; ++i)
        cout << line[i];
}

static void pr_winsize(int fd){
    struct winsize  size;
    if (ioctl(fd, TIOCGWINSZ, (char *) &size) < 0)
        cerr << "TIOCGWINSZ error";
    printf("%d rows, %d columns\n", size.ws_row, size.ws_col);
}

static void sig_winch(int signo){
    struct winsize  size;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &size) < 0)
        cerr << "TIOCGWINSZ error";
    ws_row = size.ws_row;
    ws_col = size.ws_col;
    crr_line = 1;
    dir_window_top = 1;
    dir_window_bot = ws_row - 1;
    dir_window_left  = 0;
    dir_window_right = ws_col - 3; 
    clear_screen();
}


vector<struct file_meta> read_dir(string path){
    DIR *streamp;
    struct dirent *dep;
    struct stat sb;
    struct file_meta fm;
    streamp = opendir(path.c_str());
    vector<struct file_meta> res;
    string file_name;

    file_name = path + "/" + ".";
    if (stat(file_name.c_str(), &sb) == -1) {
        cerr << "stat:" << dep->d_name << endl;
        //exit(EXIT_FAILURE);
        
    }
    fm.name = /* path + "/" + */ ".";
    fm.size = sb.st_size;
    fm.mode = sb.st_mode;
    fm.mtime = sb.st_mtime;
    
    res.push_back(fm);

    file_name = path + "/" + "..";
    if (stat(file_name.c_str(), &sb) == -1) {
        cerr << "stat:" << dep->d_name << endl;
        //exit(EXIT_FAILURE);
        
    }
    fm.name = /* path + "/" + */ "..";
    fm.size = sb.st_size;
    fm.mode = sb.st_mode;
    fm.mtime = sb.st_mtime;
    
    res.push_back(fm);

    while ((dep = readdir(streamp)) != NULL) {
        if(strcmp(dep->d_name, ".") == 0 or strcmp(dep->d_name, "..") == 0) continue;
        file_name = path + "/" + dep->d_name;
        if (stat(file_name.c_str(), &sb) == -1) {
            //cerr << "stat:" << dep->d_name << endl;
            //exit(EXIT_FAILURE);
            continue;
        }
        fm.name = /* path + "/" + */ dep->d_name;
        fm.size = sb.st_size;
        fm.mode = sb.st_mode;
        fm.mtime = sb.st_mtime;
        
        res.push_back(fm);
    }
    closedir(streamp);
    return res;
}

string mode_to_text(mode_t mode){
    string res = "";
    res += (mode & S_IFDIR) ? "d" : "-";
    res += (mode & S_IRUSR) ? "r" : "-";
    res += (mode & S_IWUSR) ? "w" : "-";
    res += (mode & S_IXUSR) ? "x" : "-";
    res += (mode & S_IRGRP) ? "r" : "-";
    res += (mode & S_IWGRP) ? "w" : "-";
    res += (mode & S_IXGRP) ? "x" : "-";
    res += (mode & S_IROTH) ? "r" : "-";
    res += (mode & S_IWOTH) ? "w" : "-";
    res += (mode & S_IXOTH) ? "x" : "-";
    return res;
}

void display_dir(vector<struct file_meta> &files){
    clear_screen();
    for(int i = dir_window_top, j = 1; i <= files.size() and i <= dir_window_bot; ++i, ++j){
        move_cursor(j, 3);
        clear_line();
        //cout << mode_to_text(files[i-1].mode) << " " << files[i-1].mtime << " " << files[i-1].size << "\t" << files[i-1].name;
        display_line(mode_to_text(files[i-1].mode) + " " + to_string(files[i-1].mtime) + " " + to_string(files[i-1].size) + " " + files[i-1].name);
    }
}

void display_footer(){
    move_cursor(ws_row, 0);
    clear_line();
    // cout << "Normal Mode : " << dir_window_top << " " << dir_window_bot << " current dir:" << crr_dir;
    display_line("Normal Mode:");
}


void copy_file(string file_name, string dest_name){
    ifstream  src(file_name, std::ios::binary);
    ofstream  dst(dest_name, std::ios::binary);
    dst << src.rdbuf();
}

void move_file(string crr_path, string file_name, string dest_path){
    copy_file(crr_path + "/" + file_name, dest_path + "/" + file_name);
    file_name = crr_path + "/" + file_name;
    remove(file_name.c_str());
}

void rename_file(string crr_path, string old_name, string new_name){
    old_name = crr_path + "/" + old_name;
    new_name = crr_path + "/" + new_name;
    copy_file(old_name, new_name);
    remove(old_name.c_str());
}

bool search(string crr_path, string name){
    vector<struct file_meta> fl;
    fl = read_dir(crr_path);
    string file_name = "";
    struct stat sb;

    // cout << crr_path << ", ";

    for(int i = 0; i < fl.size(); ++i){
        file_name = crr_path + "/" + fl[i].name;
        if(stat(file_name.c_str(), &sb) == -1){
            continue;
        }
        
        if((sb.st_mode & S_IFDIR) == 0 and fl[i].name == name){
            return true;
        }
    }
    for(int i = 0; i < fl.size(); ++i){
        if(fl[i].name == "." or fl[i].name == "..")
            continue;
        file_name = crr_path + "/" + fl[i].name;
        if(stat(file_name.c_str(), &sb) == -1)
            continue;
        if((sb.st_mode & S_IFDIR) == 0)
            continue;
        if(search(file_name, name))
            return true;
    }
    return false;
}

void delete_dir(string crr_path){
    vector<struct file_meta> fl;
    fl = read_dir(crr_path);
    string file_name = "";
    struct stat sb;

    for(int i = 0; i < fl.size(); ++i){
        if(fl[i].name == "." or fl[i].name == "..")
            continue;
        file_name = crr_path + "/" + fl[i].name;
        if(stat(file_name.c_str(), &sb) == -1){
            continue;
        }
        
        if((sb.st_mode & S_IFDIR) == 0){
            remove(file_name.c_str());
        }else{
            delete_dir(file_name);
            rmdir(file_name.c_str());
        }
    }
}


void copy_dir(string src_path, string dest_path){
    vector<struct file_meta> fl;
    fl = read_dir(src_path);
    string file_name = "";
    string dir_name  = "";
    struct stat sb;

    for(int i = 0; i < fl.size(); ++i){
        file_name = src_path + "/" + fl[i].name;
        if(stat(file_name.c_str(), &sb) == -1){
            continue;
        }
        
        if((sb.st_mode & S_IFDIR) == 0){
            copy_file(file_name, dest_path + "/" + fl[i].name);
        }else if(fl[i].name != "." and fl[i].name != ".."){
            dir_name = dest_path + "/" + fl[i].name;
            mkdir(dir_name.c_str(), 0777);
            copy_dir(file_name, dir_name);
        }
    }
}

string correct_path(string path, string crr_path){
    if(path.length() == 0) return "";
    if(path[0] == '~'){
        path[0] = '.';
    }else{
        path = crr_path + "/" + path;
    }
    return path;
}


int main(int argc, char** argv){
    char ch;
    int mode = 0;
    string command = "";
    char* buffer;

    vector<string> command_parts;
    int command_parts_idx = 0;

//    Get the current working directory:
   if ( (buffer = getcwd( NULL, 0 )) == NULL ){
      perror( "getcwd error" );
      exit(1);
   }
   else{
      crr_dir = buffer;
      free(buffer);
   }
    // crr_dir = ".";

    init_timeline();

    if (isatty(STDIN_FILENO) == 0)exit(1);
    if (signal(SIGWINCH, sig_winch) == SIG_ERR)
        cerr << "signal error";
    sig_winch(1);

    enter_raw_mode();
    files = read_dir(crr_dir);
    display_dir(files); 
    display_arrow();

    while(1){
        if(mode == 0 /* normal mode */){
            display_dir(files);
            display_arrow();
            display_footer();
            ch = cin.get();

            if(ch == KEY_UP){
                move_cursor(crr_line,0); 
                cout << " ";
                if(crr_line == 1 and dir_window_top > 1){
                    dir_window_top--;
                    dir_window_bot--;
                }
                crr_line = max(1, crr_line - 1);
            }
            else if(ch == KEY_DOWN){
                move_cursor(crr_line,0); 
                cout << " ";
                if(crr_line == ws_row - 1 and dir_window_bot < files.size()){
                    dir_window_top++;
                    dir_window_bot++;
                }
                crr_line = min(min(ws_row - 1, (int)files.size()), crr_line + 1);
            }
            else if(ch == KEY_ENTER){
                if(files[dir_window_top + (crr_line - 1) - 1].mode & S_IFDIR 
                    and files[dir_window_top + (crr_line - 1) - 1].name != "."
                ){
                    string next_dir = crr_dir + "/" + files[dir_window_top + (crr_line - 1) - 1].name;
                    struct stat sb;
                    if (stat(next_dir.c_str(), &sb) == -1) {
                        cerr << "stat_111" << endl;
                    }else{
                        crr_line = 1;
                        crr_dir = next_dir;
                        files.clear();
                        files = read_dir(crr_dir);
                        dir_window_top = 1;
                        dir_window_bot = dir_window_top + (ws_row - 1) - 1;

                        insert_into_timeline(crr_dir);
                    }
                }else if(
                    (files[dir_window_top + (crr_line - 1) - 1].mode & S_IFDIR) == 0
                ){
                    string path = crr_dir + "/" + files[dir_window_top + (crr_line - 1) - 1].name;
                    pid_t processID = fork();
                    if(processID == 0){
                        execlp("xdg-open","xdg-open", path.c_str(), NULL);
                        exit(0);
                    } 
                }
            }
            else if(ch == 'h' or ch == 'H'){
                crr_dir = ".";
                crr_line = 1;
                files.clear();
                files = read_dir(crr_dir);
                dir_window_top = 1;
                dir_window_bot = dir_window_top + (ws_row - 1) - 1;
                insert_into_timeline(crr_dir);
            }
            else if(ch == KEY_BACK){
                crr_dir += "/..";
                crr_line = 1;
                files.clear();
                files = read_dir(crr_dir);
                dir_window_top = 1;
                dir_window_bot = dir_window_top + (ws_row - 1) - 1;
                insert_into_timeline(crr_dir);
            }
            else if(ch == KEY_LEFT){
                if(tl_crr > 0){
                    --tl_crr;
                    crr_dir = timeline[tl_crr];
                    files = read_dir(crr_dir);  
                    crr_line = 1;  
                }
            }
            else if(ch == KEY_RIGHT){
                if(tl_crr < tl_len - 1){
                    ++tl_crr;
                    crr_dir = timeline[tl_crr];
                    files = read_dir(crr_dir);   
                    crr_line = 1; 
                }
            }
            else if(ch == ':'){
                mode = 1; /* command mode */
            }
            else if(ch == 'q' or ch == 'Q'){
                break;
            }
            else if(ch == 'k' or ch == 'K'){
                dir_window_left  = max(0, dir_window_left - 1);
                dir_window_right = dir_window_left + (ws_col - 3);
            }
            else if(ch == 'l' or ch == 'L'){
                dir_window_right = dir_window_right + 1;
                dir_window_left  = dir_window_right - (ws_col - 3);
            }
            else{
                // cout << (int)ch;
            }
        }else if(mode == 1 /* command mode */){
            if(command_parts.size() == 0){
                command_parts.push_back("");
                command_parts_idx = 0;
            }

            move_cursor(ws_row, 0);
            clear_line();
            display_line("Command Mode:" + command);
            
            ch = cin.get();
            if(ch == KEY_ESC){
                mode = 0;
                files = read_dir(crr_dir);
            }else if(ch == ' '){
                if(command.length() == 0 or command.back() == ' ')
                    continue;
                command.push_back(ch);
                command_parts.push_back("");
                command_parts_idx++;
            }else if(ch == KEY_BACK){
                if(command.length() == 0) continue;
                if(command.back() == ' '){
                    command_parts.pop_back();
                    command_parts_idx--;
                }else{
                    if(command_parts[command_parts_idx].length() > 0)
                        command_parts[command_parts_idx].pop_back();
                }
                command.pop_back();
            }else if(' ' < ch and ch <= '~'){
                command_parts[command_parts_idx].push_back(ch);
                command.push_back(ch);
            }else if(ch == KEY_ENTER){
/***************************Execute the command********************************/
                while(command_parts.size() > 0 and command_parts.back() == "")
                    command_parts.pop_back();
                
                if(command_parts.size() == 0){
                    continue;
                }
                
                if(command_parts[0] == "copy"){
                    if(command_parts.size() < 3){
                        command = "log: Err";
                        command_parts.clear();
                        continue;
                    }
                    string dest_path = command_parts.back();
                    dest_path = correct_path(dest_path, crr_dir);
                    for(int i = 1; i < command_parts.size() - 1; ++i){
                        string file_name = crr_dir + "/" + command_parts[i];
                        struct stat sb;
                        
                        if (stat(file_name.c_str(), &sb) == -1) {
                            continue;
                        }
                        if((sb.st_mode & S_IFDIR) == 0){
                            copy_file(file_name, dest_path + "/" + command_parts[i]);
                            command = "log: copied file";
                        }else{
                            string dir_name = dest_path + "/" + command_parts[i];
                            mkdir(dir_name.c_str(), 0777);
                            copy_dir(file_name, dir_name);
                        }
                    }
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "move"){
                    if(command_parts.size() < 3){
                        command = "log: Err";
                        command_parts.clear();
                        continue;
                    }
                    string dest_path = command_parts.back();
                    dest_path = correct_path(dest_path, crr_dir);
                    for(int i = 1; i < command_parts.size() - 1; ++i){
                        string file_name = crr_dir + "/" + command_parts[i];
                        struct stat sb;
                        if (stat(file_name.c_str(), &sb) == -1)
                            continue;
                        if((sb.st_mode & S_IFDIR) == 0){
                            move_file(crr_dir, command_parts[i], dest_path);
                            command = "log: copied file";
                        }
                    }
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "rename"){
                    if(command_parts.size() == 3){
                        rename_file(crr_dir, command_parts[1], command_parts[2]);
                        command = "log: renamed file";
                    }else
                        command = "log: error";
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "create_file"){
                    if(command_parts.size() != 3){
                        command = "log: Err";
                    }else{
                        string file_path = command_parts[2];
                        file_path = correct_path(file_path, crr_dir);
                        string file_name = file_path + "/" + command_parts[1];
                        ofstream  dst(file_name, std::ios::binary);
                    }
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "create_dir"){
                    if(command_parts.size() != 3){
                        command = "log: Err";
                    }else{
                        string dir_path = command_parts[2];
                        dir_path = correct_path(dir_path, crr_dir);
                        string dir_name = dir_path + "/" + command_parts[1];
                        mkdir(dir_name.c_str(), 0777);
                        command = "";
                    }
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "delete_file"){
                    if(command_parts.size() != 2){
                        command = "log: Err";
                    }else{
                        string file_path = command_parts[1];
                        file_path = correct_path(file_path, crr_dir); 
                        int rc = remove(file_path.c_str());
                        if(rc)
                            command = "log: cannot remove";
                        else
                            command = "log: removed";
                        
                    }
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "delete_dir"){
                    if(command_parts.size() != 2){
                        command = "log: Err";
                    }else{
                        string dir_path = command_parts[1]; 
                        dir_path = correct_path(dir_path, crr_dir);
                        struct stat sb;
                        if(stat(dir_path.c_str(), &sb) != -1){
                            delete_dir(dir_path);
                            rmdir(dir_path.c_str());
                            command = "log: deleted";
                        }
                    }
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "goto"){
                    if(command_parts.size() != 2){
                        command = "log: Err";
                        continue;
                    }else{
                        string dir_path = command_parts[1]; 
                        dir_path = correct_path(dir_path, crr_dir);
                        crr_dir = dir_path;
                        files = read_dir(crr_dir);
                        display_dir(files);
                    }
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "search"){
                    command = search(crr_dir, command_parts[1]) ? "found" : "not found";
                    command_parts.clear();
                }
                
                
                else if(command_parts[0] == "q" or command_parts[0] == "Q"){
                    break;
                }
                
                
                else{
                    command = "";
                    command_parts.clear();
                }
            }
        }
    }
    move_cursor(1, 0);
    clear_screen();
    exit_raw_mode();
    return 0;
}