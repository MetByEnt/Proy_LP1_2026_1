#include <windows.h>
#include <wingdi.h>
#include <stdbool.h>
#include "Firmware_Semaforo.h"
#define SPEED 3
#define BORDERSIZE 8
#define NGE 11

int vSize=600; //tamaño de la ventana
int modoAutomatico=0;
enum EstadoSemaforo estado = EN;
struct Configuracion configuracion = {2,1,4};
HWND GUIelements[NGE]={0};

//Estructura De Carro
struct Carro{
    int x;
    int y;
    int vertical;
    int horizontal;
    struct Carro* carroAnterior;
};
struct Carro* listaCarros=NULL;
void CrearCarro( int x, int y, int vertical , int horizontal, HWND padre){
    struct Carro* carro = malloc(sizeof(struct Carro));
    carro->x=x;carro->y=y;
    carro->vertical=vertical;
    carro->horizontal=horizontal;

    if(horizontal!=0){
        for(struct Carro* buffer=listaCarros;buffer!=NULL;buffer=buffer->carroAnterior){
            if(buffer!=carro & buffer->horizontal==carro->horizontal){
                int distancia = (buffer->x-carro->x)*carro->horizontal;
                if(distancia>0 && distancia<60){
                    return;
                }
            }
        }
    }
    if(vertical!=0){
        for(struct Carro* buffer=listaCarros;buffer!=NULL;buffer=buffer->carroAnterior){
            if(buffer!=carro & buffer->vertical==carro->vertical){
                int distancia = (buffer->y-carro->y)*carro->vertical;
                if(distancia>0 && distancia<60){
                    return;
                }
            }
        }
    }

    carro->carroAnterior=listaCarros;
    listaCarros=carro;

}
void MoverCarro(struct Carro* carro){
    if(carro->horizontal!=0){
        //Verifica si tiene que parar
        if((estado==E1 | estado==E2) & ((carro->x<200 & carro->horizontal==1) | (carro->x>400 & carro->horizontal==-1))){
            if((carro->x>192 & carro->horizontal==1) | (carro->x<408 & carro->horizontal==-1)){
                return;
            }
            for(struct Carro* buffer=listaCarros;buffer!=NULL;buffer=buffer->carroAnterior){
                if(buffer!=carro & buffer->horizontal==carro->horizontal){
                    int distancia = (buffer->x-carro->x)*carro->horizontal;
                    if(distancia>0 && distancia<60){
                        return;
                    }
                }
            } 
        }       
        //si amarillo
        if((estado==E4 | estado==EN) & (carro->x<400 & carro->x>200) ){
            carro->x += carro->horizontal*SPEED*1;
            return;
        }
    }
    if(carro->vertical!=0){
        if((estado==E3 | estado==E4) & ((carro->y<200 & carro->vertical==1) | (carro->y>400 & carro->vertical==-1))){
            if((carro->y>192 & carro->vertical==1) | (carro->y<408 & carro->vertical==-1)){
                return;
            }
            for(struct Carro* buffer=listaCarros;buffer!=NULL;buffer=buffer->carroAnterior){
                if(buffer!=carro & buffer->vertical==carro->vertical){
                    int distancia = (buffer->y-carro->y)*carro->vertical;
                    if(distancia>0 && distancia<60){
                        return;
                    }
                }
            }
        }
        if((estado==E2 | estado==EN) & carro->y<400 & carro->y>200){//si amarillo

            carro->y += carro->vertical*SPEED*1;
            return;
        }
    }
    carro->x += carro->horizontal*SPEED;
    carro->y += carro->vertical*SPEED;
}


//Controlador De Mensajes de la Ventana Principal
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    static HBITMAP fondo;
    static HBITMAP playbmp;
    static HBITMAP stopbmp;
    switch (uMsg)
    {
        case WM_COMMAND:
   			switch(LOWORD(wParam)){
                case 0:
                    DestroyWindow(hwnd);
                break;
                case 1:
                    CrearCarro(252,0,1,0,hwnd);
                break;
                case 2:
                    CrearCarro(0,312,0,1,hwnd);
                break;
                case 3:
                    CrearCarro(312,600,-1,0,hwnd);
                break;
                case 4:
                    CrearCarro(600,252,0,-1,hwnd);
                break;
                case 5: 
                    HWND bauto = (HWND)lParam;
                    if(modoAutomatico){
                        modoAutomatico=0;
                        SendMessage(bauto,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)playbmp);
                    }else{
                        modoAutomatico=1;
                        SendMessage(bauto,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)stopbmp);
                    }

                break;
                case 6:
                    if(HIWORD(wParam) == EN_CHANGE){
                        char text[16];
                        GetWindowText((HWND)lParam,text,16);
                        configuracion.tiempo_verde = (float)atof(text);
                    }
                break;
                case 7:
                    if(HIWORD(wParam) == EN_CHANGE){
                        char text[16];
                        GetWindowText((HWND)lParam,text,16);
                        configuracion.tiempo_amarillo = (float)atof(text);
                    }
                break;
                case 8:
                    if(HIWORD(wParam) == EN_CHANGE){
                        char text[16];
                        GetWindowText((HWND)lParam,text,16);
                        configuracion.tiempo_para_estado_nocturno = (float)atof(text);
                    }
                break;
            }
   		break;
        case WM_CREATE:
            fondo = (HBITMAP)LoadImage(NULL, "Calle.bmp",IMAGE_BITMAP, 600, 600, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            playbmp =(HBITMAP)LoadImage(NULL, "Sprites\\play.bmp",IMAGE_BITMAP, 44, 44, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            stopbmp =(HBITMAP)LoadImage(NULL, "Sprites\\stop.bmp",IMAGE_BITMAP, 44, 44, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            break;
	    case WM_DESTROY:
	        PostQuitMessage(0);
	        return 0;
        break;
        case WM_NCHITTEST:
            int x= (short) LOWORD(lParam);
            int y= (short) HIWORD(lParam);
            RECT rect;
            GetWindowRect(hwnd, &rect);
            
            {
            int ax= (x-rect.left)*600/vSize;
            int ay= (y-rect.top)*600/vSize;
            if(ax>536 & ax<584 & ay>20 & ay<68){//Boton de Cerrar
                return HTCLOSE;
            }
            }

            BOOL left   = x < rect.left   + BORDERSIZE;
            BOOL right  = x >= rect.right  - BORDERSIZE;
            BOOL top    = y < rect.top    + BORDERSIZE;
            BOOL bottom = y >= rect.bottom - BORDERSIZE;

            if (top    && left)  return HTTOPLEFT;
            if (top    && right) return HTTOPRIGHT;
            if (bottom && left)  return HTBOTTOMLEFT;
            if (bottom && right) return HTBOTTOMRIGHT;
            if (top)             return HTTOP;
            if (bottom)          return HTBOTTOM;
            if (left)            return HTLEFT;
            if (right)           return HTRIGHT;


    		return HTCAPTION;
		break;
        case WM_NCLBUTTONDOWN:
            if (wParam == HTCLOSE) {
                DestroyWindow(hwnd);
                return 0;
            }
        break;
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 255, 255));   
            SetBkMode(hdcStatic, TRANSPARENT);      

            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        case WM_SIZING://Cuando Cambia el Tamaño que mantenga el ratio de un cuadrado
            RECT* vRect=(RECT*)lParam;
            int width = vRect->right- vRect->left;
            int height = vRect->bottom - vRect->top;
            switch(wParam){
                case WMSZ_LEFT:
                case WMSZ_RIGHT:
                    vRect->bottom=vRect->top + width;
                    vSize = width;
                break;

                case WMSZ_TOP:
                case WMSZ_BOTTOM:
                    vRect->right=vRect->left + height;
                    vSize= height;
                break;
                case WMSZ_TOPLEFT:
                case WMSZ_TOPRIGHT:
                    vRect->top=vRect->bottom - width;
                    vSize = width;
                break;
                case WMSZ_BOTTOMLEFT:
                case WMSZ_BOTTOMRIGHT:
                    vRect->left=vRect->right - height;
                    vSize = height;
                break;
            }
            static int f=0;
            static RECT guiRects[NGE];
            if(f==0){
                for(int i=0;i<NGE;i++){
                    GetWindowRect(GUIelements[i],&guiRects[i]);
                    ScreenToClient(hwnd, (LPPOINT)&guiRects[i].left);
                    ScreenToClient(hwnd, (LPPOINT)&guiRects[i].right);
                }
                f=1;
            }else{
                float sizeRatio = (float)vSize/600;

                for(int i=0;i<NGE;i++){
                    SetWindowPos(GUIelements[i],NULL,guiRects[i].left*sizeRatio,guiRects[i].top*sizeRatio,(guiRects[i].right-guiRects[i].left)*sizeRatio,(guiRects[i].bottom-guiRects[i].top)*sizeRatio,SWP_NOZORDER);
                }
            }

            return TRUE;
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//La Ventana de la Simulacion
int main(){
    //Creacion de la Ventana
	HINSTANCE hInst;
	WNDCLASSA wc = {0};
    wc.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "aea";
    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "RegisterClass failed", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    HWND hwnd=CreateWindowEx(0,"aea","xd",WS_POPUP | WS_BORDER | WS_VISIBLE | WS_EX_COMPOSITED ,100,100,600,600,NULL,NULL,hInst,NULL);
    
    //Elementos De la GUI
    
    GUIelements[0]=CreateWindowEx(0,"button","Crear Carro Aqui",WS_VISIBLE | WS_CHILD,390,20,120,30,hwnd,(HMENU)1,NULL,NULL);
    GUIelements[1]=CreateWindowEx(0,"button","Crear Carro Aqui",WS_VISIBLE | WS_CHILD,10,190,120,30,hwnd,(HMENU)2,NULL,NULL);
    GUIelements[2]=CreateWindowEx(0,"button","Crear Carro Aqui",WS_VISIBLE | WS_CHILD,390,550,120,30,hwnd,(HMENU)3,NULL,NULL);
    GUIelements[3]=CreateWindowEx(0,"button","Crear Carro Aqui",WS_VISIBLE | WS_CHILD,470,190,120,30,hwnd,(HMENU)4,NULL,NULL);    

    GUIelements[4] = CreateWindowEx(0,"static",NULL,WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY,160,140,44,44,hwnd,(HMENU)5,NULL,NULL);
        HBITMAP autobmp =(HBITMAP)LoadImage(NULL, "Sprites\\play.bmp",IMAGE_BITMAP, 44, 44, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        SendMessage(GUIelements[4],STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)autobmp);
    
    GUIelements[5] = CreateWindowEx(0,"static","Tiempo en Verde",WS_CHILD | WS_VISIBLE,12,440,150,30,hwnd,NULL,NULL,NULL);
    GUIelements[6] = CreateWindowEx(0,"static","Tiempo en Amarillo",WS_CHILD | WS_VISIBLE,12,478,150,30,hwnd,NULL,NULL,NULL);
    GUIelements[7] = CreateWindowEx(0,"static","Tiempo para estado Nocturno",WS_CHILD | WS_VISIBLE,12,516,150,30,hwnd,NULL,NULL,NULL);
    GUIelements[8] = CreateWindowEx(0,"edit","2",WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT |ES_NUMBER,174,440,50,30,hwnd,(HMENU)6,NULL,NULL);
    GUIelements[9] = CreateWindowEx(0,"edit","1",WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT |ES_NUMBER,174,478,50,30,hwnd,(HMENU)7,NULL,NULL);
    GUIelements[10] = CreateWindowEx(0,"edit","4",WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT |ES_NUMBER,174,516,50,30,hwnd,(HMENU)8,NULL,NULL);
    

    
    //Variables Para Dibujar
    HDC pantalla = GetDC(hwnd);
    HDC     hdcBack   = CreateCompatibleDC(pantalla);
    HBITMAP hbmBack   = CreateCompatibleBitmap(pantalla, 600, 600);
    HBRUSH fondobrush = CreatePatternBrush((HBITMAP)LoadImage(NULL, "Sprites\\Calle.bmp",IMAGE_BITMAP, 600, 600, LR_LOADFROMFILE | LR_CREATEDIBSECTION));
    RECT DimensionesV; 
    GetClientRect(hwnd,&DimensionesV);
    SelectObject(hdcBack, hbmBack);
    
    HBITMAP carrobmph = (HBITMAP)LoadImage(NULL, "Sprites\\carro.bmp",IMAGE_BITMAP, 52, 36, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    HBITMAP carrobmpv= (HBITMAP)LoadImage(NULL, "Sprites\\carrov.bmp",IMAGE_BITMAP, 36, 52, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    HBITMAP semaforobmp = (HBITMAP)LoadImage(NULL, "Sprites\\semaforo.bmp",IMAGE_BITMAP, 144, 76, LR_LOADFROMFILE | LR_CREATEDIBSECTION);



    //Ciclo Principal    
    bool running=true;
    ShowWindow(hwnd,1);
    MSG msg;
    time_t timeAuto;
    srand(time(NULL)); 
    while(running){

        //Dibujar Carros
        FillRect(hdcBack, &DimensionesV, fondobrush);
        for(struct Carro* buffer = listaCarros; buffer !=NULL; buffer=buffer->carroAnterior){
            HDC hdb = CreateCompatibleDC(hdcBack);
            if(buffer->horizontal!=0){
                SelectObject(hdb,carrobmph);
                StretchBlt(hdcBack,buffer->x,buffer->y,buffer->horizontal*52,36,hdb,0,0,52,36,SRCCOPY);
            }
            if(buffer->vertical!=0){
                SelectObject(hdb,carrobmpv);
                StretchBlt(hdcBack,buffer->x,buffer->y,36,buffer->vertical*(52),hdb,0,0,36,52,SRCCOPY);
            }
            DeleteDC(hdb);
        }


        HDC hdsemaforo = CreateCompatibleDC(hdcBack);
        SelectObject(hdsemaforo,semaforobmp);
        COLORREF colorS2 = RGB(64,64,64);
        COLORREF colorS1 = RGB(64,64,64);
        int offset2=0;
        int offset1=0;
        switch(estado){
            case E1:
                colorS1 = RGB(0,255,0);
                colorS2 = RGB(252,0,0);
            break;
            case E2:
                colorS1 = RGB(255,255,0);
                offset1 = 12;
                colorS2 = RGB(252,0,0);
            break;
            case E3:
                colorS1 = RGB(255,0,0);
                offset1 = 24;
                colorS2 = RGB(0,255,0);
                offset2=24;
            break;
            case E4:
                colorS1 = RGB(255,0,0);
                offset1 = 24;
                colorS2 = RGB(255,255,0);
                offset2=12;
                break;
            case EN:
                colorS1 = RGB(255,255,0);
                offset1 = 12;
                colorS2 = RGB(255,255,0);
                offset2=12;
            break;
        }
        HBRUSH hBrush = CreateSolidBrush(RGB(64,64,64));
        RECT poscolorSemaforo1 = {36,52,44,60};
        RECT poscolorSemaforo2 = {84,8,92,16};
        for(int i=0;i<3;i++){
            FillRect(hdsemaforo,&poscolorSemaforo1,hBrush);
            FillRect(hdsemaforo,&poscolorSemaforo2,hBrush);
            poscolorSemaforo1.left+=12;
            poscolorSemaforo1.right+=12;
            poscolorSemaforo2.top+=12;
            poscolorSemaforo2.bottom+=12;
        }
        poscolorSemaforo1.left-=36-offset2;
        poscolorSemaforo1.right-=36-offset2;
        poscolorSemaforo2.top -= 36-offset1;
        poscolorSemaforo2.bottom-= 36-offset1;
        DeleteObject(hBrush);
        hBrush = CreateSolidBrush(colorS2);
        FillRect(hdsemaforo,&poscolorSemaforo1,hBrush);
        DeleteObject(hBrush);
        hBrush = CreateSolidBrush(colorS1);
        FillRect(hdsemaforo,&poscolorSemaforo2,hBrush);
        DeleteObject(hBrush);

        TransparentBlt(hdcBack,212,204,76,68,hdsemaforo,0,0,76,68,RGB(0,0,0));
        TransparentBlt(hdcBack,332,316,68,76,hdsemaforo,76,0,68,76,RGB(0,0,0));
        DeleteDC(hdsemaforo);
        StretchBlt(pantalla,0,0,vSize,vSize,hdcBack,0,0,600,600,SRCCOPY);
        

        //Buscar Mensajes (Botones, etc)
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT){
                running=false;
            }else{
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }


        //Modo automatico (Crea Carros cada cierto Tiempo)
        if(modoAutomatico){
            if(difftime(time(NULL),timeAuto)>0.25){//Que intente crear un carro cada 1 segundo
                struct Carro* temp;
                switch(rand()%4){
                    case 0:
                        CrearCarro(252,0,1,0,hwnd);
                    break;
                    case 1:
                        CrearCarro(0,312,0,1,hwnd);
                    break;
                    case 2:
                        CrearCarro(312,600,-1,0,hwnd);
                    break;
                    case 3:
                        CrearCarro(600,252,0,-1,hwnd);
                    break;
                }
                timeAuto = time(NULL);
            }
        }else{
            timeAuto = time(NULL);
        }


        Sleep(4); // 62 fps xd

        
        //Revisar Carros fuera de limites 
        {
            struct Carro* carroBuffer=listaCarros;
            struct Carro* aux = NULL;
            while(carroBuffer!=NULL){
                
                MoverCarro(carroBuffer);
                
                if(carroBuffer->x>600| carroBuffer->x < 0 | carroBuffer->y >600 | carroBuffer->y < 0){
                    if(aux==NULL){
                        free(carroBuffer);
                        carroBuffer=NULL;
                        listaCarros=NULL;
                    }else{
                        aux->carroAnterior = carroBuffer->carroAnterior;
                        free(carroBuffer);
                        carroBuffer = aux->carroAnterior;
                    }
                }else{
                    aux=carroBuffer;
                    carroBuffer=carroBuffer->carroAnterior;
                }
            }
        }
        //Revisa el Trafico
        int traficoHorizontal=0;
        int traficoVertical=0;
        for(struct Carro* buffer=listaCarros; buffer!=NULL;buffer= buffer->carroAnterior){
            if(buffer->horizontal==1 & buffer->x<200){
                traficoHorizontal++;
            }
            if(buffer->horizontal==-1 & buffer->x>400){
                traficoHorizontal++;
            }
            if(buffer->vertical==1 & buffer->y<200){
                traficoVertical++;
            }
            if(buffer->vertical==-1 & buffer->y>400){
                traficoVertical++;
            }
        }
        estado = ChecarEstado(estado,configuracion,traficoVertical,traficoHorizontal);
    }

    return 0;
}
