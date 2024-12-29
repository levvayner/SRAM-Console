#include "Console.hpp"
#include "../Editor/Editor.hpp"
#include "UI/UI.h"
#define CONTEXT_CONSOLE "console"
extern Console console;
extern Editor editor;
extern UI ui;
DueFlashStorage dueFlashStorage;
void listFiles(const char * path, int indent,  char*  flags);
int endsWith(const char *str, const char *suffix);

void clear(commandRequest request){
    console.EraseCursor();
    console.clear();    
    console.SetPosition(0,0);
    console.SetScrollPosition(0);
    return;
}

void cd(commandRequest request){
    //console.AdvanceCursor(true);
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
    
    listFiles(strlen(request.args) > 0 ? request.args : console.path().c_str(), 0, request.flags);
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
    console.print("Are you sure you want to delete "); console.print(request.args); console.println("?");
    console.print("Y/n");
    while(true)
    {
        char k = keyboard.getKey();
        if(k == 0) continue;
        if(k == 'Y' || k == 'y'){
            Serial.print("Delete yes");
            return;
        } 
        if(k == 'N' || k == 'n'){
            Serial.print("Delete no");
            return;
        } 


    }

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
    //console.AdvanceCursor(true);
    if(!SD.exists(request.args)){        
        //println("Cannot Edit. Invalid path");
        console.println("Creating new file");
        //return 0;
    }
    console.end();
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
    //console.AdvanceCursor(true);
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
    console.print("Running ["); console.print(requestPath.c_str()); console.println("]");
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
    
    console.end();  //drop due interrupt
    auto result = startApp(FLASH1, console);
    //failed to start app
    console.run();
    uint8_t color = console.GetColor();
    console.SetColor(Color::RED);
    console.print("Failed to start app: ");
    console.println(returnResultText(result));
    console.SetColor(color);
    return;
}

void commandExit(commandRequest request){
    console.end();
    ui.begin();

}

void dumpData(commandRequest request){
    uint16_t pos = console.GetDataPos();
    char nextChar = ' ';
    if(pos <=  0) return;
    Serial.println("Dumping data contents... ");
    //     return;
    for(int idx = 0; idx < pos;idx++){
        
        nextChar = programmer.ReadByte(1 << 19 | idx);
        if((nextChar >= 32 && nextChar < 127) || nextChar == 10)
            Serial.print(nextChar);
    }
    programmer.ReadByte(0x0); // unset bit 19 so screen accesses video ram
}

void registerConsoleCommands(){
    commands.registerCommand(CONTEXT_CONSOLE,"clear", "",clear);
    commands.registerCommand(CONTEXT_CONSOLE,"cd", "",cd);
    commands.registerCommand(CONTEXT_CONSOLE,"ls", "lr",ls);
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
    commands.registerCommand(CONTEXT_CONSOLE, "dump", "", dumpData);

}


char fileSize[32];
char fileDate[32];
ScratchArea _scratch;
uint64_t size = 0;
void listFiles(const char * path, int indent,  char*  flags)
{
    File rootFile = SD.open(path);
    bool longFormat = strchr(flags, 'l') != nullptr;
    bool recursive = strchr(flags, 'r') != nullptr;
    if(!rootFile){
        return;
    }
    File entry;    
    while(true){
        uint8_t fgColor = console.GetColor();
        uint8_t color = fgColor;
        entry = rootFile.openNextFile();
        
        if(!entry) break;
        memset(_scratch.bytes, PS2_KEY_SPACE, sizeof(_scratch));
        memset(fileSize, 0, sizeof(fileSize));
        if(entry.isDirectory()){
            sprintf(fileSize, "%s", "<dir>");
            color = Color::YELLOW;
        }
        else{
            sprintf(fileSize, "%lu%s",  entry.size(), " bytes");            
            size+= entry.size();
            if(endsWith(entry.name(), ".BIN")){
                color = Color::BRICK;
            }
        }
        
        
        if(longFormat){
            sprintf(_scratch.text,"%*s%s%*s%s%*s",indent*2,"", entry.name(), 20 - strlen(entry.name()), "", fileSize, 15 - strlen(fileSize), "");
            sprintf(_scratch.text + strlen(_scratch.text), "%s", "date here");
        } else{ //just the filename - short
            sprintf(_scratch.text,"%s", entry.name());
        }
        
        //write(_scratch.text);
        //auto pos = console.GetPosition();
        console.SetColor(color);
        if(console.GetPosition().x + (strlen(_scratch.text) * graphics.settings.charWidth) > graphics.settings.screenWidth)
            console.write(10); // if text would overflow, go to next line
        console.write(_scratch.text);
        console.SetColor(fgColor);
        // programmer.WriteBytes(1 << 19 | console.GetDataPos(), _scratch.bytes, strlen(_scratch.text));
        // graphics.drawText(pos.x, pos.y, _scratch.text, console.GetColor(), console.GetBackgroundColor());
        if(longFormat || (entry.isDirectory() && recursive))
            console.println();
        else{
            console.write("  ");
        }
        
        if(entry.isDirectory() && recursive){
            listFiles(entry.name(), indent + 1, flags);
        }
        entry = entry.openNextFile();
    }    
    entry.close();
    rootFile.close();
    if(indent == 0){        
        console.write(10);
        
        console.println("-----------------------------------------------------");
        console.print("Total Size:     "); 
        if(size > (1024*1024)){
            console.print( (float)size / (float)(1024*1024)); 
            console.print("MB");
        }
        else if(size > 1024){
            console.print(size / (1024)); 
            console.print(".");
            console.print(size % (1024));
            console.print("KB");
        }
        else{
            console.print(size);
            console.println(" bytes");
        }
        size = 0;
    }
}

int endsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}