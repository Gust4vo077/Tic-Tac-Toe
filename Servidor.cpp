#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>
#define PORT 12346
#define QUEUE 20

int conn;
int erroMensagem(char buffer[1024]);        // função que verifica se a mensagem é no padrão (A|B|C & 1|2|3) da matriz
int Ganhou(int posicaoVetor,int valor);     // função que garante que se há um ganhador seja informado
int SetMatriz(char buffer[1024],int valor); // seta o valor 1 ou 0 para oponentes na matriz e verifica se é válido a posição 
void desenharGanhadorFinal();               // printa uma tela para no final para o ganhador ou perdedor
void desenhar();                            // printa uma tela para mostrar o jogo da velha na tela 
#define SIMBOLO(x) ( x == 0 )? " " : ( ( x==1 )? "X" : "O" ) // macro para fazer a substituição de X ou O no print da tela 
int matriz[9]={0,0,0,0,0,0,0,0,0};          // vetor que vai fazer a abstração do jogo da velha 

int main() {
    // Define o socket
    int ss = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT);

    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Faz o bind com o socket criado
    if(bind(ss, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr))==-1) {
        perror("bind");
        exit(1);
    }
    //listen for connections on a socket
    if(listen(ss, QUEUE) == -1) {
        perror("listen");
        exit(1);
    }

    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    /// Successful return of non-negative descriptor, error Return-1
    conn = accept(ss, (struct sockaddr*)&client_addr, &length);
    if( conn < 0 )
    {
        perror("connect");
        exit(1);
    }
   
    char buffer[1024];
    memset(buffer, 0 ,sizeof(buffer)); // seta 0 para todas a posiçoes do vetor buffer
    while(true) 
      {
        
        // função que recebece o buffer    
        recv(conn, buffer, sizeof(buffer),0); 
        // compara buffer com exit para sair    
        if(strcmp(buffer,"exit\n")==0)
            break;

        int ganhador =SetMatriz(buffer,1);
        // condições que trata conforme o retorno da função SetMatriz
        
        if(ganhador == 1)// adversário ganhou trata a saida e tela final
         {
          system("clear");
          desenharGanhadorFinal();
          char aux[]=" Voce perdeu :(\n";
          fputs(aux,stdout);
          close(conn);
          close(ss);
          return 1;
         } 
        else if( ganhador == 2) // Voce ganhou trata a saida e tela final
         {
          system("clear");
          desenharGanhadorFinal();
          char aux[]=" Voce ganhou :)\n";
          fputs(aux,stdout);
          close(conn);
          close(ss);
          return 1;
         }

        
	while(true)
         {
           memset(buffer, 0, sizeof(buffer));
           system("clear");
           // Desenha na tela a matriz com os X e O
           desenhar();
           fgets(buffer, sizeof(buffer), stdin); 
           // compara buffer com exit para sair        
           if(strcmp(buffer,"exit\n")==0)
              break;

           int confirma = erroMensagem(buffer);
           // condições que trata conforme o retorno da função erroMensagem 
	   
           if(confirma==0)// Se a mensagem for escrita de forma errada
             {
              char aux[]=" 'Mensagem digita Errado Siga a forma pedida' :Aperte enter para continuar\n";
              fputs(aux,stdout);
              fgets(aux,sizeof(aux),stdin);
	     }
          
           else if (confirma==2)// Se a mensagem estiver escritade forma errada em espaços em brancos
             {   
              char aux[]="'A palavra tem linhas em branco Siga a forma pedida' :Aperte enter para continuar\n";        
	      fputs(aux,stdout);
              fgets(aux,sizeof(aux),stdin);
             }	
           else // Mesagem escrita na forma correta 
             {
              int reposta=SetMatriz(buffer,2); // como é o Servidor oponente do Cliente Sera escolhido 'O' para ele e será representado como 2 no vetor
 
              // condições que trata conforme o retorno da função SetMatriz 
             
              if( reposta == -1 )// entra se a posição ja estiver sido escolhida
                {
             
                 char aux[]=" Essa posição ja foi Escolhida anteriomente \n";
                 fputs(aux,stdout);
                 fgets(aux, sizeof(aux), stdin);           
                }
              else if( reposta == 1 )// adversário ganhou trata a saida e tela final 
                {
                 system("clear");
                 desenharGanhadorFinal();
                 char aux[]=" Voce perdeu :(\n";
                 fputs(aux,stdout);
                 send(conn,buffer, strlen(buffer),0);
                 close(conn);
                 close(ss);
                 return 1;
                }
              else if( reposta == 2 ) // Voce ganhou trata a saida e tela final
                {
                 system("clear");
                 desenharGanhadorFinal();
                 char aux[]=" Voce ganhou :)\n";
                 fputs(aux,stdout);
                 send(conn,buffer, strlen(buffer),0);
                 close(conn);
                 close(ss);
                 return 1;  
                } 
              else if(reposta == 0) // niguem ganhou ou perdeu o jogo continua       
                 break;
             }    
         
         }
        
        
        if(strcmp(buffer,"exit\n")==0)
            break;
        // função que mandar para o Cliente pelo buffer
        send(conn, buffer, strlen(buffer),0);
        system("clear");//limpa a tela 
        // desenha com a posição atualizada da escolha do Servidor 
        desenhar();
        memset(buffer, 0, sizeof(buffer)) ;

      }
    // executa o fechamento para sair do programa
    close(conn); 
    close(ss);
    return 0;
}

int erroMensagem(char buffer[1024]) {
 char aux[]="ABC123";
 // verifica se  o codigo esta escrito na forma A|B|C e 1|2|3 na posição do vetor  1 e 2 
 if( buffer[0]==aux[0] | buffer[0]==aux[1] | buffer[0]==aux[2] ){
  if( buffer[1]==aux[3] | buffer[1]==aux[4] | buffer[1]==aux[5] )
   return 1;
 }
 // verifica se a espaço em branco 
 else if( (int)buffer[0]== 0 | (int)buffer[1]== 0 | (int)buffer[2]== 0 ){
  return 2;
 }
 
 return 0;
 
}

int SetMatriz(char buffer[1024],int valor){
    
    // usa a tecnica de subtrair da tabela ascii o A = 65, possíveis 0,1 e 2 e multiplica por 3 por ser um matriz 3x3
    int Linha = ((int)buffer[0]-65)*3;
    
    // usa a tecnica de subtrair da tabela ascii o 1 = 49, possíveis 0,1 e 2 
    int Coluna =((int)buffer[1]-49);
    
    // faz a soma para achar posiçoões vetor 
    int posicaoVetor = Linha+Coluna;
    
    // verica se a posição do vetor ja esta ocupada
    if(matriz[posicaoVetor]!=0)
     return -1;

    // já verfica se tem ganhador
    int reposta=Ganhou(posicaoVetor,valor);
 
 return reposta;  
}

int Ganhou(int posicaoVetor,int valor){
   
  matriz[posicaoVetor]=valor;
    
  if( ( (matriz[0]==matriz[4])&(matriz[0]==matriz[8]) ) | ( (matriz[4]==matriz[2])&(matriz[4]==matriz[6])  ) ){ // confere a diagonal
    if(matriz[4]== 1)
        return 1;//X
    else if(matriz[4] == 2)    
        return 2;//O
  }       
 for(int i=0;i<3;i++){// confere colunas

    if( (matriz[i]==matriz[i+3]) & (matriz[i]==matriz[i+6]) ){
       if( matriz[i] == 1 )    
        return 1;//X
       else if(matriz[i] == 2)    
        return 2;//O
    } 
  
  }
 for(int i=0;i<3;i=i+3){// confere linhas
    
    if( (matriz[i]==matriz[i+1])&(matriz[i]==matriz[i+2] ) ){
     if( matriz[i] == 1 )    
       return 1;//X
     else if(matriz[i] == 2)  
       return 2;//O
    } 
    
  }    
   return 0;// niguem ganhou 
}

void desenhar(){
  std::cout<<"\n\n      ____.                                   .___                         .__  .__            "<<std::endl;
  std::cout<<"     |    | ____   ____   ____              __| _/____        ___  __ ____ |  | |  |__  _____   "<<std::endl;
  std::cout<<"     |    |/  _ \\ / ___\\ /  _ \\            / __ |\\__  \\       \\  \\/ //  __\\|  | |  |  \\ \\__  \\  "<<std::endl;
  std::cout<<" /\\__|    (  <_> ) /_/  >  <_> )          / /_/ | / __ \\_      \\   /\\  ___/|  |_|   Y |  / __ \\_"<<std::endl;
  std::cout<<" \\________|\\____/\\___  / \\____/           \\_____|(______/       \\_/  \\___> |____/___| | (______/"<<std::endl;
  std::cout<<"                /_____/                                                                               "<<std::endl;
  std::cout<<"\n\n\n\n                          1   2   3"<<std::endl;    
  std::cout<<"                      A   "<< (SIMBOLO(matriz[0]) )<< " | " <<(SIMBOLO(matriz[1]) )<<" | "<<(SIMBOLO(matriz[2]))<<std::endl;
  std::cout<<"                         -----------"<<std::endl;
  std::cout<<"                      B   "<< (SIMBOLO(matriz[3]) )<< " | " <<(SIMBOLO(matriz[4]) )<<" | "<<(SIMBOLO(matriz[5]))<<std::endl;
  std::cout<<"                         -----------"<<std::endl;
  std::cout<<"                      C   "<< (SIMBOLO(matriz[6]) )<< " | " <<(SIMBOLO(matriz[7]) )<<" | "<<(SIMBOLO(matriz[8]))<<std::endl;
  
  std::cout<<"\n\nOrdem para mandar a Mensagem Seria LinhaColuna exemplo A3 ou C1"<<std::endl;  
}

void desenharGanhadorFinal(){
  std::cout<<"\n\n      ____.                                   .___                         .__  .__            "<<std::endl;
  std::cout<<"     |    | ____   ____   ____              __| _/____        ___  __ ____ |  | |  |__  _____   "<<std::endl;
  std::cout<<"     |    |/  _ \\ / ___\\ /  _ \\            / __ |\\__  \\       \\  \\/ //  __\\|  | |  |  \\ \\__  \\  "<<std::endl;
  std::cout<<" /\\__|    (  <_> ) /_/  >  <_> )          / /_/ | / __ \\_      \\   /\\  ___/|  |_|   Y |  / __ \\_"<<std::endl;
  std::cout<<" \\________|\\____/\\___  / \\____/           \\_____|(______/       \\_/  \\___> |____/___| | (______/"<<std::endl;
  std::cout<<"                /_____/                                                                               "<<std::endl;
  std::cout<<"\n\n\n\n                           1   2   3"<<std::endl;    
  std::cout<<"                      A   "<< (SIMBOLO(matriz[0]) )<< " | " <<(SIMBOLO(matriz[1]) )<<" | "<<(SIMBOLO(matriz[2]))<<std::endl;
  std::cout<<"                         -----------"<<std::endl;
  std::cout<<"                      B   "<< (SIMBOLO(matriz[3]) )<< " | " <<(SIMBOLO(matriz[4]) )<<" | "<<(SIMBOLO(matriz[5]))<<std::endl;
  std::cout<<"                         -----------"<<std::endl;
  std::cout<<"                      C   "<< (SIMBOLO(matriz[6]) )<< " | " <<(SIMBOLO(matriz[7]) )<<" | "<<(SIMBOLO(matriz[8]))<<std::endl;
    
}