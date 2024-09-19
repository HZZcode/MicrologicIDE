#include "aboutdialog.h"
#include "MicrologicIDE.h"
#include "ui_MicrologicIDE.h"
#include <fstream>
#include <iostream>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QColorDialog>
#include <QFontDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QProcess>
#include <QTextLayout>

std::string exepath="";

MicrologicIDE::MicrologicIDE(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MicrologicIDE)
{
    currentContent="";

    ui->setupUi(this);
    ui->textEdit->setDocumentTitle("untitled -- MicrologicIDE");
    setWindowTitle(ui->textEdit->documentTitle());

    connect(ui->newFileAction, &QAction::triggered, this, &MicrologicIDE::newFile);
    connect(ui->openFileAction, &QAction::triggered, this, &MicrologicIDE::openFile);
    connect(ui->saveFileAction, &QAction::triggered, this, &MicrologicIDE::saveFile);
    connect(ui->exitNotepadAction, &QAction::triggered, this, &MicrologicIDE::exitNotepad);
    connect(ui->saveAsFileAction, &QAction::triggered, this, &MicrologicIDE::saveAsFile);
    connect(ui->aboutAction, &QAction::triggered, this, &MicrologicIDE::doForAboutAction);

    connect(ui->getHelpAction, &QAction::triggered, this, &MicrologicIDE::seeHelp);
    connect(ui->dateAction, &QAction::triggered, this, &MicrologicIDE::setDateTime);

    connect(ui->copyAction, &QAction::triggered, ui->textEdit, &QTextEdit::copy);
    connect(ui->cutAction, &QAction::triggered, ui->textEdit, &QTextEdit::cut);
    connect(ui->pasteAction, &QAction::triggered, ui->textEdit, &QTextEdit::paste);
    connect(ui->undoAction, &QAction::triggered, ui->textEdit, &QTextEdit::undo);
    connect(ui->selectAllAction, &QAction::triggered, ui->textEdit, &QTextEdit::selectAll);

    connect(ui->runAction, &QAction::triggered, this, &MicrologicIDE::runFile);

    connect(ui->textEdit, &QTextEdit::textChanged, this, &MicrologicIDE::markError);

}

MicrologicIDE::~MicrologicIDE()
{
    delete ui;
}

//新建文件
void MicrologicIDE::newFile(void)
{
    if (ui->textEdit->document()->isModified()) //编辑区内容是否改变
    {
        qDebug() << "document is modified";
        switch (requestIsSave())               //是否要保存
        {
            case QMessageBox::Yes :            //保存
                saveDocumentText();

                if (!currentfileName.isEmpty())
                {
                    currentfileName.clear();
                }

                setDocumentTitile("untitled -- MicrologicIDE");
                ui->textEdit->document()->clear();
                break;
            case QMessageBox::No :
                setDocumentTitile("untitled -- MicrologicIDE");
                ui->textEdit->document()->clear();
                break;
            case QMessageBox::Cancel :
                qDebug() << "取消";
                break;
            default:
                break;
        }
    }
    else
    {
        qDebug() << "document is not modified";
    }
}

void MicrologicIDE::openFile(void)
{
    if (ui->textEdit->document()->isModified()) //编辑区内容是否改变
    {
        saveDocumentText();
    }
    getDocumentText();
    setDocumentTitile(currentfileName);
}

void MicrologicIDE::saveFile(void)
{
    if (ui->textEdit->document()->isModified() || !currentfileName.isEmpty())
    {
        QString tmpFileName = saveDocumentText();
        setDocumentTitile(tmpFileName);
        currentfileName = tmpFileName;
    }
}

void MicrologicIDE::saveAsFile()
{
    if (ui->textEdit->document()->isModified())
    {
        QString tmpFileName = currentfileName;
        currentfileName.clear();
        saveDocumentText();
        currentfileName = tmpFileName;
    }
}


//退出
void MicrologicIDE::exitNotepad()
{
    if (ui->textEdit->document()->isModified())
    {
        switch (requestIsSave())
        {
            case QMessageBox::Yes :
                saveDocumentText();
                this->close();
                break;
            case QMessageBox::No :
                this->close();
                break;
            case QMessageBox::Cancel :
                qDebug() << "取消";
                break;
            default:
                break;
        }
    }
}

void MicrologicIDE::doForAboutAction()
{
    AboutDialog about;
    about.exec();
}

void MicrologicIDE::setDateTime()
{
    ui->textEdit->append(QDateTime::currentDateTime().toString());
}

void MicrologicIDE::seeHelp(void)
{
    QUrl url("https://github.com/HZZcode/MicrologicIDE");
    QDesktopServices::openUrl(url);
}


QString MicrologicIDE::saveDocumentText(void)
{
    QString fileName;
    QFile file;
    QTextStream *out;

    if (currentfileName.isEmpty())   //如果当前没有打开的文件，就将内容保存新建的文件中，否则保存到当前打开的文件中
    {
        fileName = QFileDialog::getSaveFileName(this, tr("保存文件"), QDir::currentPath(), tr("Micrologic文件(*.mcl)"));
    }
    else
    {
        fileName = currentfileName;
    }

    file.setFileName(fileName);
    out = new QTextStream(&file);
    if (file.open(QIODevice::WriteOnly))   //只写模式打开
    {
        *out << ui->textEdit->document()->toPlainText(); //将textEdit编辑区域内容写入文件中
        file.close();
    }

    return fileName;
}

void MicrologicIDE::getDocumentText()
{
    QString fileName;
    QFile file;
    QTextStream *in;

    fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), QDir::currentPath(), tr("Micrologic文件(*.mcl)"));
    currentfileName = fileName;
    file.setFileName(fileName);
    in = new QTextStream(&file);
    if (file.open(QIODevice::ReadOnly))   //只写模式打开
    {
        ui->textEdit->setText(in->readAll());
        file.close();
    }
}

int MicrologicIDE::requestIsSave()
{
    int answer =  QMessageBox::question(this, tr("MicrologicIDE"), tr("是否将更改保存？"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    return answer;
}


void MicrologicIDE::setDocumentTitile(QString title)
{
    ui->textEdit->setDocumentTitle(title);
    setWindowTitle(ui->textEdit->documentTitle());
}

void MicrologicIDE::runFile()
{
    QByteArray b=(QString::fromStdString("start /d \""+exepath+"\" Micrologic.exe ")+currentfileName).toLatin1();
    system(b.data());
}

std::vector<std::string> MicrologicIDE::content(){
    std::string text=ui->textEdit->document()->toPlainText().toStdString();
    std::vector<std::string> lines;
    while(1){
        int t=text.find("\n");
        if(t!=std::string::npos){
            lines.push_back(text.substr(0,t));
            text=text.substr(t+1,text.size());
        }
        else{
            lines.push_back(text);
            break;
        }
    }
    return lines;
}

void MicrologicIDE::mark(std::vector<int> errorList={})
{
    std::string text;
    std::vector<std::string> lines=content();
    for(int k:errorList) if(lines.size()>=k+1) lines[k]="<span style=\"text-decoration: underline; text-decoration-color: red; white-space: pre;\">"+lines[k]+"</span>";
    for(std::string& line:lines) text+=(line+"<br>");
    text=text.substr(0,text.length()-4);
    ui->textEdit->setText(text.c_str());
}

void MicrologicIDE::unmark()
{
    std::string text=ui->textEdit->document()->toPlainText().toStdString();
    ui->textEdit->setText(text.c_str());
}

void MicrologicIDE::markError(){
    if(currentContent!=ui->textEdit->document()->toPlainText().toStdString()){
        currentContent=ui->textEdit->document()->toPlainText().toStdString();
        QTextCursor cursor=ui->textEdit->textCursor();
        int pos=cursor.position();
        unmark();
        std::vector<int> errorList;
        auto grammar=grammarCheck(content());
        for(int i=0;i<content().size();i++) if(!grammar[i]) errorList.push_back(i);
        mark(errorList);
        cursor=ui->textEdit->textCursor();
        cursor.setPosition(pos);
        ui->textEdit->setTextCursor(cursor);
    }
}

int countInput(std::vector<std::string> lines){
    int ans=0;
    for(int i=0;i<lines.size();i++){
        std::string line=lines[i];
        std::stringstream ss(line);
        std::string s;
        std::vector<std::string> args;
        while (std::getline(ss, s, ' ')) {
            args.push_back(s);
        }
        if(args.size()>0) if(args[0]=="input:") ans++;
    }
    return ans;
}

int countOutput(std::vector<std::string> lines){
    int ans=0;
    for(int i=0;i<lines.size();i++){
        std::string line=lines[i];
        std::stringstream ss(line);
        std::string s;
        std::vector<std::string> args;
        while (std::getline(ss, s, ' ')) {
            args.push_back(s);
        }
        if(args.size()>0) if(args[0]=="output:") ans++;
    }
    return ans;
}

std::map<std::string,std::pair<int,int>> MicrologicIDE::findMods(std::vector<std::string> lines){
    std::map<std::string,std::pair<int,int>> mods;
    for(int i=0;i<lines.size();i++){
        std::string line=lines[i];
        std::stringstream ss(line);
        std::string s;
        std::vector<std::string> args;
        while (std::getline(ss, s, ' ')) {
            args.push_back(s);
        }
        if(args.size()>0){
            if (args[0] == "path" && count(line.begin(), line.end(), '\"') >= 2) {
                int x = line.find("\""), y = line.rfind("\"");
                std::string f = line.substr(x + 1, y - x - 1);
                path = f;
            }
            else if (args[0] == "path" && args.size() == 2) {
                path = args[1];
            }
            else if(args[0]=="mod"&&args.size()==3){
                try{
                    std::vector<std::string> lines1;
                    std::ifstream fin;
                    fin.open(path+args[2], std::ios::out | std::ios::in);
                    bool b=fin.good();
                    char fcmd[256] = "";
                    while (fin.getline(fcmd, 256)) {
                        lines1.push_back(fcmd);
                    }
                    std::map<std::string,std::pair<int,int>> mods1=findMods(lines1);
                    if(b) mods.insert({args[1],{countInput(lines1),countOutput(lines1)}});
                    mods.insert(mods1.begin(),mods1.end());
                }catch(...){}
            }
        }
    }
    return mods;
}

std::vector<bool> MicrologicIDE::grammarCheck(std::vector<std::string> lines)
{
    std::vector<bool> ans(lines.size());
    std::vector<bool> Ls; //line order->is wide
    std::vector<bool> ins; //input line order->is wide
    std::vector<bool> outs; //output line order->is wide
    std::map<std::string,int> modBlock; //mod name->block count
    std::map<std::string,std::pair<int,int>> mods=findMods(lines); //modname->{input count,output count}
    for(std::string s:{"N","A","R","T","C","P"}) modBlock.insert({s,0});
    for(int i=0;i<lines.size();i++){
        std::string line=lines[i];
        std::stringstream ss(line);
        std::string s;
        std::vector<std::string> args;
        while (std::getline(ss, s, ' ')) {
            args.push_back(s);
        }
        if(args.size()==0) ans[i]=true;
        else if(args[0]=="end"||args[0]=="clear"){
            ans[i]=args.size()==1;
        }
        else if((args[0]=="line"||args[0]=="wline")&&args.size()==1){
            ans[i]=true;
            Ls.push_back(args[0]=="wline");
        }
        else if((args[0]=="line"||args[0]=="wline")&&args.size()==2){
            ans[i]=isNumber(args[1]);
            if(isNumber(args[1])) for(int j=0;j<atoi(args[1].c_str());j++) Ls.push_back(args[0]=="wline");
        }
        else if((args[0]=="line"||args[0]=="wline")&&args.size()>=3){
            ans[i]=false;
        }
        else if((args[0]=="N"||args[0]=="T")&&args.size()==3){
            if(isNumber(args[1])&&isNumber(args[2])){
                if(atoi(args[1].c_str())<Ls.size()&&atoi(args[2].c_str())<Ls.size()&&Ls[atoi(args[1].c_str())]==0&&Ls[atoi(args[2].c_str())]==0){
                    ans[i]=true;
                    modBlock[args[0]]++;
                }
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if((args[0]=="A"||args[0]=="R")&&args.size()==4){
            if(isNumber(args[1])&&isNumber(args[2])&&isNumber(args[3])){
                if(atoi(args[1].c_str())<Ls.size()&&atoi(args[2].c_str())<Ls.size()&&atoi(args[3].c_str())<Ls.size()&&Ls[atoi(args[1].c_str())]==0&&Ls[atoi(args[2].c_str())]==0&&Ls[atoi(args[3].c_str())]==0){
                    ans[i]=true;
                    modBlock[args[0]]++;
                }
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="C"&&args.size()==6){
            if(isNumber(args[1])&&isNumber(args[2])&&isNumber(args[3])&&isNumber(args[4])&&isNumber(args[5])){
                if(atoi(args[1].c_str())<Ls.size()&&atoi(args[2].c_str())<Ls.size()&&atoi(args[3].c_str())<Ls.size()&&atoi(args[4].c_str())<Ls.size()&&atoi(args[5].c_str())<Ls.size()&&Ls[atoi(args[1].c_str())]==0&&Ls[atoi(args[2].c_str())]==0&&Ls[atoi(args[3].c_str())]==0&&Ls[atoi(args[4].c_str())]==0&&Ls[atoi(args[5].c_str())]==1){
                    ans[i]=true;
                    modBlock[args[0]]++;
                }
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="P"&&args.size()==6){
            if(isNumber(args[1])&&isNumber(args[2])&&isNumber(args[3])&&isNumber(args[4])&&isNumber(args[5])){
                if(atoi(args[1].c_str())<Ls.size()&&atoi(args[2].c_str())<Ls.size()&&atoi(args[3].c_str())<Ls.size()&&atoi(args[4].c_str())<Ls.size()&&atoi(args[5].c_str())<Ls.size()&&Ls[atoi(args[1].c_str())]==1&&Ls[atoi(args[2].c_str())]==0&&Ls[atoi(args[3].c_str())]==0&&Ls[atoi(args[4].c_str())]==0&&Ls[atoi(args[5].c_str())]==0){
                    ans[i]=true;
                    modBlock[args[0]]++;
                }
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="set"&&args.size()==3){
            if(isNumber(args[1])&&isNumber(args[2])){
                if(atoi(args[1].c_str())<Ls.size()&&Ls[atoi(args[1].c_str())]==0&&(args[2]=="0"||args[2]=="1")) ans[i]=true;
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="set"&&args.size()==6){
            if(isNumber(args[1])&&isNumber(args[2])&&isNumber(args[3])&&isNumber(args[4])&&isNumber(args[5])){
                if(atoi(args[1].c_str())<Ls.size()&&Ls[atoi(args[1].c_str())]==1&&(args[2]=="0"||args[2]=="1")&&(args[3]=="0"||args[3]=="1")&&(args[4]=="0"||args[4]=="1")&&(args[5]=="0"||args[5]=="1")) ans[i]=true;
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="check"&&args.size()==1){
            ans[i]=true;
        }
        else if(args[0]=="check"&&args.size()==2){
            if(isNumber(args[1])){
                if(atoi(args[1].c_str())<Ls.size()) ans[i]=true;
                else ans[i]=false;
            }
        }
        else if((args[0]=="input:"||args[0]=="output:")&&args.size()==2){
            if(isNumber(args[1])){
                if(atoi(args[1].c_str())<Ls.size()){
                    ans[i]=true;
                    if(args[0]=="input:") ins.push_back(Ls[atoi(args[1].c_str())]);
                    if(args[0]=="output:") outs.push_back(Ls[atoi(args[1].c_str())]);
                }
                else ans[i]=false;
            }
        }
        else if(args[0]=="input"&&args.size()==3){
            if(isNumber(args[1])&&isNumber(args[2])){
                if(atoi(args[1].c_str())<ins.size()&&(args[2]=="0"||args[2]=="1")&&ins[atoi(args[1].c_str())]==0) ans[i]=true;
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="input"&&args.size()==6){
            if(isNumber(args[1])&&isNumber(args[2])&&isNumber(args[3])&&isNumber(args[4])&&isNumber(args[5])){
                if(atoi(args[1].c_str())<ins.size()&&(args[2]=="0"||args[2]=="1")&&(args[3]=="0"||args[3]=="1")&&(args[4]=="0"||args[4]=="1")&&(args[5]=="0"||args[5]=="1")&&ins[atoi(args[1].c_str())]==1) ans[i]=true;
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="output"&&args.size()==1){
            ans[i]=true;
        }
        else if(args[0]=="output"&&args.size()==2){
            if(isNumber(args[1])){
                if(atoi(args[1].c_str())<outs.size()) ans[i]=true;
                else ans[i]=false;
            }
        }
        else if((args[0]=="tick"||args[0]=="tick!")&&args.size()==1){
            ans[i]=true;
        }
        else if((args[0]=="tick"||args[0]=="tick!"||args[0]=="speed")&&args.size()==2){
            ans[i]=isNumber(args[1]);
        }
        else if(args[0]=="path"&&count(line.begin(),line.end(),'\"')>=2){
            ans[i]=true;
        }
        else if(args[0]=="path"&&args.size()==2){
            ans[i]=true;
        }
        else if(args[0]=="open"&&args.size()==2){
            ans[i]=true;
        }
        else if(args[0]=="open"&&count(line.begin(),line.end(),'\"')>=2){
            ans[i]=true;
        }
        else if(args[0]=="mod"&&args.size()==3){
            modBlock.insert({args[1],0});
            ans[i]=true;
        }
        else if(args[0]=="block"&&args.size()>=3){
            if(mods.count(args[1])!=0){
                if(args.size()==2+mods[args[1]].first+mods[args[1]].second) ans[i]=true;
                else ans[i]=false;
                modBlock[args[1]]++;
            }
            else ans[i]=false;
        }
        else if(args[0]=="inspect"&&args.size()==3){
            if(mods.count(args[1])!=0||args[1]=="N"||args[1]=="A"||args[1]=="R"||args[1]=="T"||args[1]=="C"||args[1]=="P"){
                if(isNumber(args[2])&&atoi(args[2].c_str())<modBlock[args[1]]) ans[i]=true;
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="echo"){
            ans[i]=true;
        }
        else if(args[0]=="@echo"&&args.size()==2){
            if(isNumber(args[1])){
                if(args[1]=="0"||args[1]=="1") ans[i]=true;
                else ans[i]=false;
            }
            else ans[i]=false;
        }
        else if(args[0]=="lang"&&args.size()==2){
            ans[i]=std::count(langs.begin(),langs.end(),args[1])==1;
        }
        else if(args[0]=="neko"&&args.size()==1){
            ans[i]=true;
        }
        else ans[i]=false;
    }
    return ans;
}

void MicrologicIDE::on_textEdit_textChanged()
{

}

