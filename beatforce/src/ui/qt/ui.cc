#include <qapplication.h>
#include <qpushbutton.h>




extern "C" int UI_Init(int argc,char **argv)
{
    QApplication beatforce( argc, argv );
    QPushButton hello( "Hello world!", 0 );
    hello.resize( 100, 30 );
    beatforce.setMainWidget( &hello );
    hello.show();
    beatforce.exec();
   
}


extern "C" int UI_Main()
{
    
}

extern "C" int THEME_Init()
{

}

extern "C" int FILEWINDOW_Init()
{


}

extern "C" int MAINWINDOW_Open()
{

}

extern "C" int CONFIGWINDOW_Open()
{


}













