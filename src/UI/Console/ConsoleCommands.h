#include "Console.hpp"
#include "../Editor/Editor.hpp"
#define CONTEXT_CONSOLE "console"
extern Console console;
extern Editor editor;
DueFlashStorage dueFlashStorage;
void listFiles(const char * path, bool recursive, int indent,  char*  flags);

void clear(commandRequest request){
    console.EraseCursor();
    console.clear();    
    console.SetPosition(0,0);
    console.SetScrollPosition(0);
    return;
}

void cd(commandRequest request){
    console.AdvanceCursor(true);
    String args = request.args;
    String pathToOpen = args;
    char buf[128];
    memset(buf,0,sizeof(buf));

    args.trim(); //get rid of spaces
    pathToOpen.trim();
    if(args.startsWith("/")){
        sprintf(buf,pathToOpen.c_str(), pathToOpen.length());
    }
    else{
        Serial.print("Relative");        
        String pathToOpen = console.path(); //relative from current      
       
        String workingPart = "";
        while(true){
            int endIdx = args.indexOf("/");
            Serial.print("Found end idx at "); Serial.println(endIdx);
            if(endIdx <= 0) {
                if(args.length() > 0){
                    Serial.print("Adding filename: "); Serial.println(args.c_str());
                    if(strcmp(args.c_str(), "..") == 0){
                        //go up one dir
                        int trimIdx = pathToOpen.lastIndexOf("/");
                        pathToOpen = (trimIdx == 0) ? "/" : pathToOpen.substring(0, pathToOpen.lastIndexOf("/") - 1);
                        Serial.print("up path");
                    }
                    else if(strcmp(args.c_str(), ".") == 0){
                        Serial.print("no change");
                    }else
                        pathToOpen += args;
                }

                Serial.print("Loop result: ");Serial.println(pathToOpen.c_str());
                sprintf(buf,pathToOpen.c_str(), pathToOpen.length());
                break; //done
            }
            //get chunk
            workingPart = args.substring(0,endIdx);
            args = args.substring(workingPart.length());

            Serial.print("Working part: "); Serial.println(workingPart.c_str());
            Serial.print("Remaining args: "); Serial.print(args.c_str());
            if(strcmp(workingPart.c_str(), ".") == 0){
                //nothing to do, stay in the same dir
                Serial.print("same path");
            }
            else if(strcmp(workingPart.c_str(), "..") == 0){
                //go up one dir
                pathToOpen = pathToOpen.substring(0, pathToOpen.lastIndexOf("/") - 1);
                Serial.print("up path");
            }
            else {
                //add to path
                pathToOpen += workingPart;
                Serial.print("adding "); Serial.println(workingPart.c_str());
            }
        }
    }
    Serial.print("Opening file: ["); Serial.print(buf);Serial.println("]");
    if(strcmp(buf, "/") == 0 || SD.exists(buf)){
            //File f = SD.open(buf);
            //sprintf(buf,"/%s",f.name());
            //f.close();
            console.setPath(buf);    
    }   
    else{
        
        auto color = console.GetColor();
        console.SetColor(Color::RED);
        console.print("Invalid path provided: "); console.println(buf);
        console.SetColor(color);
    }
    console.SetEchoMode(true);
}

void ls(commandRequest request){
    //check if we are in sd volume
    console.SetEchoMode(false);
    if(!console.path().startsWith("/")){        
        console.println("Cannot LS. Invalid path");
        return;
    }
    
    listFiles(strlen(request.args) > 0 ? request.args : console.path().c_str(), true, 0, request.flags);
    console.SetEchoMode(true); 
}

void df(commandRequest request){
    console.printDiskInfo();
}

void cat(commandRequest request){
    //check if we are in sd volume
    console.SetEchoMode(false);
    String requestPath = request.args;
    requestPath.trim();
    bool addSeperator = true;
    if(requestPath.startsWith("/")) addSeperator = false;    
    
    
    char buf[128];
    sprintf(buf, "%s%s%s", addSeperator ? "" : console.path().c_str(), addSeperator ? "/" : "" , requestPath.c_str());
    Serial.print("Executing cat on file "); Serial.println(buf);
    File f = SD.open(buf);
    if(f){
        uint8_t buffer[72];
        int readBytes = 0;
        while(true){
            if(!f.available())
                break;
            readBytes = f.readBytes(buffer,sizeof(buffer));
            if(readBytes == 0)
                continue;
            
            console.write(buffer,readBytes);
            console.AdvanceCursor(true);
            
        }
        f.close();
    }
    console.SetEchoMode(true);
}

void rm(commandRequest request){

}

void rmdir(commandRequest request){

}

void paint(commandRequest request){
    Bitmap img;
    String requestPath = request.args;
    requestPath.trim();
    bool addSeperator = true;
    if(requestPath.startsWith("/")) addSeperator = false;       
    char buf[128];
    sprintf(buf, "%s%s%s", addSeperator ? "" : console.path().c_str(), addSeperator ? "/" : "" , requestPath.c_str());
    if(!SD.exists(buf)){
        console.write("Error: file not found!"); 
        console.write(buf);
        console.println();
        return;
    }
    img.drawFile(buf, 0, 0);
    console.SetPosition(0,0);
}

void edit(commandRequest request){
    console.AdvanceCursor(true);
    if(!SD.exists(request.args)){        
        //println("Cannot Edit. Invalid path");
        console.println("Creating new file");
        //return 0;
    }
    console.stop();
    editor.open(request.args); 
    editor.run();
    
    //done editing, save memory back to file
    //editor.save();
}

void run(commandRequest request){

}

void drawShape(commandRequest request){
    int coordsLength  = 0;
    int coords[6];
    if(strcasecmp(request.name,"line")  == 0){
        coordsLength = console.getCoords(request.args, coords);
        if(coordsLength != 4) return;
        Serial.println("Drawing line");
        graphics.drawLine(coords[0], coords[1], coords[2], coords[3],console.GetColor());
    }
    else if(strcasecmp(request.name,"tri")  == 0){
        coordsLength = console.getCoords(request.args, coords);
        if(coordsLength != 6) return;
        graphics.drawTriangle(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5],console.GetColor());

    }
    else if(strcasecmp(request.name,"rect")  == 0){
        coordsLength = console.getCoords(request.args, coords);
        if(coordsLength != 4)return;
        graphics.drawRectangle(coords[0], coords[1], coords[2], coords[3],console.GetColor());

    }
    else if(strcasecmp(request.name,"cir")  == 0){
        coordsLength = console.getCoords(request.args, coords);
        if(coordsLength != 3) return;
        graphics.drawCircle(coords[0], coords[1], coords[2],console.GetColor());

    } else{
        Serial.println("Shape not implemented");
    }
}
void printText(commandRequest request){
    auto pos = console.GetPosition();
    graphics.drawText(pos.x, pos.y, request.args, console.GetColor(), console.GetBackgroundColor());
}

void printHelp(commandRequest request){
    char buf[128] = {0};
    String text;
    console.SetEchoMode(false);
    console.AdvanceCursor(true);
    for(int idx = 0; idx < commands.commandCount(); idx++){
        auto cmd = commands.getCommand(idx);
        sprintf(buf,"%-*s%-10s%s%s", 10,cmd->name, cmd->context, strlen(cmd->flags) > 0 ? " - flags: " : "", cmd->flags);        
        sprintf(buf,"%-*s", 35 ,buf);        
        text += buf;
        if(idx % 2 == 1) text += '\n';
    }
    console.write(text.c_str());
    console.SetEchoMode(true);
}

void runProgram(commandRequest request){
    char buf[256];
    char filename[65];
    String requestPath = request.args;
    requestPath.trim();
    //byte bufVerify[256];
    console.print("Running ["); Serial.console(requestPath.c_str()); console.println("]");
    sprintf(filename, "%s%s%s", console.path().c_str(),  console.path().length()  == 1 ? "" : "/" , requestPath.c_str());
    if(!SD.exists((const char *)filename))
    {
        Serial.println("File not found!");
        return;
    }
    console.println("Loading file into ROM...");
    File program = SD.open(filename);
    uint32_t  memAddr = 0x0;
    int bytesRead = 0;//, bytesWritten = 0;
    while(true){
        //uint8_t data = 0;
        bytesRead = program.read(buf,sizeof(buf));
        if(!program.available()) break;
        if(bytesRead == 0) break;
        if(!dueFlashStorage.write(memAddr,(byte*)buf,bytesRead)){
            console.println("Error occured writing data");
            return;
        }
        memAddr+= bytesRead;        
    } 
    console.println("Starting app");
    
    console.stop();  //drop due interrupt
    startApp(FLASH1);
    Serial.print("Returned control");
    return;
}

void commandExit(commandRequest request){
    console.stop();

}
void registerConsoleCommands(){
    commands.registerCommand(CONTEXT_CONSOLE,"clear", "",clear);
    commands.registerCommand(CONTEXT_CONSOLE,"cd", "",cd);
    commands.registerCommand(CONTEXT_CONSOLE,"ls", "l",ls);
    commands.registerCommand(CONTEXT_CONSOLE,"df", "",df);
    commands.registerCommand(CONTEXT_CONSOLE,"cat", "",cat);
    commands.registerCommand(CONTEXT_CONSOLE,"rm", "",rm);
    commands.registerCommand(CONTEXT_CONSOLE,"rmdir", "",rmdir);
    commands.registerCommand(CONTEXT_CONSOLE,"paint", "",paint);
    commands.registerCommand(CONTEXT_CONSOLE,"edit", "",edit);
    commands.registerCommand(CONTEXT_CONSOLE,"run", "",run);
    commands.registerCommand(CONTEXT_CONSOLE,"rect", "",drawShape);
    commands.registerCommand(CONTEXT_CONSOLE,"tri", "",drawShape);
    commands.registerCommand(CONTEXT_CONSOLE,"line", "",drawShape);
    commands.registerCommand(CONTEXT_CONSOLE,"cir", "",drawShape);
    commands.registerCommand(CONTEXT_CONSOLE,"print", "",printText);
    commands.registerCommand(CONTEXT_CONSOLE,"help", "",printHelp);
    commands.registerCommand(CONTEXT_CONSOLE,"run", "",runProgram);
    commands.registerCommand(CONTEXT_CONSOLE,"exit", "",commandExit);

}


char fileSize[32];
char fileDate[32];
ScratchArea _scratch;
void listFiles(const char * path, bool recursive, int indent,  char*  flags)
{
    File rootFile = SD.open(path);
    if(!rootFile){
        return;
    }
    File entry;    
    while(true){
        entry = rootFile.openNextFile();
        if(!entry) break;
        memset(_scratch.bytes, 0, sizeof(_scratch));
        memset(fileSize, 0, sizeof(fileSize));
        if(entry.isDirectory())
            sprintf(fileSize, "%s", "<dir>");
        else
            sprintf(fileSize, "%lu%s",  entry.size(), " bytes");
        
        sprintf(_scratch.text,"%s%*s %s", entry.name(), 20 - strlen(entry.name()), "", fileSize);

        if(strstr("l",flags) != nullptr){
            sprintf(_scratch.text, "%s%*s", _scratch.text, 20, "date here");
        }
        //write(_scratch.text);
        auto pos = console.GetPosition();
        programmer.WriteBytes(1 << 19 | console.GetDataPos(), _scratch.bytes, strlen(_scratch.text));
        graphics.drawText(pos.x, pos.y, _scratch.text, console.GetColor(), console.GetBackgroundColor());
        console.write(10,true);
       
        entry = entry.openNextFile();
        
        if(entry.isDirectory()){
            listFiles(entry.name(), recursive, indent + 1, flags);
        }
    }    
    entry.close();
    rootFile.close();

    
}