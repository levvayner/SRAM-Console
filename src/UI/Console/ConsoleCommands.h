#include "Console.hpp"
#include "../Editor/Editor.hpp"
#define CONTEXT_CONSOLE "console"
extern Console console;
extern Editor editor;
DueFlashStorage dueFlashStorage;
void listFiles(const char * path, bool recursive, int indent,  char*  flags);

void clear(commandRequest request){
    console.clear();
    console.SetPosition(0);
    return;
}

void cd(commandRequest request){
    char buf[128];
    memset(buf,0,sizeof(buf));
    if(strncasecmp("../", request.args, 4) == 0 ||  strncasecmp("..", request.args, 3) == 0 ){
        //go up one dir
        if(console.path()->lastIndexOf('/') > 0)     
            console.setPath(console.path()->substring(0,console.path()->lastIndexOf('/') - 1).c_str());
//            _path = _path.substring(0,_path.lastIndexOf('/') - 1);
        else
            console.setPath("/");
    }
    else if(SD.exists(request.args)){
        File f = SD.open(request.args);
        sprintf(buf,"/%s",f.name());
        f.close();
        console.setPath(buf);        
    }
        
    else{
        console.print("Invalid path provided: "); console.println(request.args);
    }
}

void ls(commandRequest request){
    //check if we are in sd volume
    console.AdvanceCursor(true);
    if(!console.path()->startsWith("/")){        
        console.println("Cannot LS. Invalid path");
        return;
    }
    
    listFiles(strlen(request.args) > 0 ? request.args : console.path()->c_str(), true, 0, request.flags);
}

void cat(commandRequest request){
    //check if we are in sd volume
    console.AdvanceCursor(true);
    String requestPath = request.args;
    requestPath.trim();
    bool addSeperator = true;
    if(requestPath.startsWith("/")) addSeperator = false;    
    
    
    char buf[128];
    sprintf(buf, "%s%s%s", addSeperator ? "" : console.path()->c_str(), addSeperator ? "/" : "" , requestPath.c_str());
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
    sprintf(buf, "%s%s%s", addSeperator ? "" : console.path()->c_str(), addSeperator ? "/" : "" , requestPath.c_str());
    img.drawFile(buf, 0, 0);
}

void edit(commandRequest request){
    console.AdvanceCursor(true);
    if(!SD.exists(request.args)){        
        //println("Cannot Edit. Invalid path");
        console.println("Creating new file");
        //return 0;
    }
    editor.open(request.args); 
    editor.run();
    //done editing, save memory back to file
    editor.save();
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
    char buf[256];
    for(int idx = 0; idx < commands.commandCount(); idx++){
        auto cmd = commands.getCommand(idx);
        auto pos = console.GetPosition();
        sprintf(buf,"%s%s%s", cmd->name, strlen(cmd->flags) > 0 ? " - flags: " : "", cmd->flags);
        graphics.drawText(pos.x, pos.y,(const char*) buf, console.GetColor());
        console.AdvanceCursor(true);
    }
}

void runProgram(commandRequest request){
    char buf[256];
    char filename[36];
    //byte bufVerify[256];
    Serial.print("Running ["); Serial.print(request.args); Serial.println("]");
    sprintf(filename, "%s%s%s", console.path()->c_str(),  console.path()->length()  == 1 ? "" : "/" , request.args);
    if(!SD.exists((const char *)filename))
    {
        Serial.println("File not found!");
        return;
    }
    File program = SD.open(filename);
    uint32_t  memAddr = 0x0;
    int bytesRead = 0;//, bytesWritten = 0;
    while(true){
        //uint8_t data = 0;
        bytesRead = program.read(buf,sizeof(buf));
        if(!program.available()) break;
        if(bytesRead == 0) break;
        if(!dueFlashStorage.write(memAddr,(byte*)buf,bytesRead)){
            Serial.println("Error occured writing data");
            return;
        }
        memAddr+= bytesRead;
        
    }
    Serial.println("Done loading binary at 0x8000");
    
    
    
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
    commands.registerCommand(CONTEXT_CONSOLE,"ls", "",ls);
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


char lsfileName[32];
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
        memset(lsfileName, 0, sizeof(lsfileName));
        ltoa(entry.size(),(char*)lsfileName,10);
        sprintf(_scratch.text,"%s %*s %s", entry.name(), 20 - strlen(entry.name()), "", entry.isDirectory() ? "<dir>" : lsfileName);
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