/*
MEMO:
Wiki - Winsock (WINdows SOCKet) est une bibliothèque logicielle pour Windows dont
le but est d'implémenter une interface de programmation inspirée de Berkeley sockets.
Elle prend notamment en charge l'envoi et la réception des paquets de données sur des réseaux TCP/IP.
(il existe 4type de socket)
Socket: il faut utiliser la socket stream propre a TCP

###########POUR LES TEST#################################################################

Pour tester le serveur en local: MENU DEMARRER>CMD>TELNET>open localhost 8888.
(Installation du client telnet http://technet.microsoft.com/fr-fr/library/cc771275(v=ws.10).aspx#bkmk_installVista)

Pour tester le client avec www.Google.fr faire -> "GET / HTTP/1.1\r\n\r\n";
###########SOCKET WINSOCK#################################################################

>>>>>WINDOWS
(Sous windows) Pour faire fonctionner l'ide codeblock avec le code faut tjrs (si je change d'ordi)
Faire ca (Ajout de DLL). Project => build options => linker settings => link library, puis ajouter
"winsock2.h" dans C:\Program Files\CodeBlocks\MinGW\include : winsock2.h
"libws2_32.a" dans C:\Program Files\CodeBlocks\MinGW\lib : libws2_32.a

PS:Ca c o k ou mais en vrai inutile je crois #pragma comment(lib, "ws2_32.lib")
###########GESTION DES THREADS############################################################

>>>>>WINDOWS
Pareil pour les threads :Pr utiliser <pthread.h> -> bibliothèque libpthread.a
ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.exe
dezipper la biblio - Mettre les .h ds C:\Program Files\CodeBlocks\MinGW\include
et les .a ds C:\Program Files\CodeBlocks\MinGW\lib
Puis les dll ds le bin
Voir le ConfigWin.pdf

>>>>>>>>>POUR linux
Dans Project => build options => linker settings => other link option, ajouoter -lpthreads
##########################################################################################
*/

#if defined (WIN32) || defined (_WIN32)

    #include <winsock2.h>
    //#pragma comment(lib,"ws2_32.lib") //Winsock Library -- Peut être supprimable

#elif defined (linux) || defined (_POSIX_VERSION) || defined (_POSIX2_C_VERSION)\
 || defined (_XOPEN_VERSION)

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <errno.h>
    #include <netdb.h>
    #include <string.h>

#endif

//#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SERVER_LISTEN_PORT 8888
#define CLIENT_LISTEN_PORT 80
#define NELEM(a) (sizeof(a)/sizeof*(a))
#define MAXCONNEXQUEUE 3
#define IP "173.194.66.94"
//#define BUFFSIZE 32

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SERVER_SOCKADRR_IN;

/*boolean RESQUEST_SHUTDOWN_SERVER = 0 ;
boolean *p_RESQUEST_SHUTDOWN_SERVER ;
p_RESQUEST_SHUTDOWN_SERVER = &RESQUEST_SHUTDOWN_SERVER;*/

//###RAPPEL_Structures_IPv4_AF_INET_SOCKETS####//
//#Dans struct sockaddr on a ceci:
//#unsigned short    sa_family;
//#char              sa_data[14];
//#
//#Dans struct sockaddr_in on a ceci:
//#short            sin_family;   AF_INET, AF_INET6
//#unsigned short   sin_port;     le port
//#struct in_addr   sin_addr;     une STRUCTURE contenant ce qui est en bas...
//#char             sin_zero[8];
//#
//#In_addr est une STRUCTURE contenant des STRUCTURES...
//#In struct in_addr {
//#  union
//#  {
//#    struct
//#    {
//#      u_char s_b1,s_b2,s_b3,s_b4;
//#    }
//#    S_un_b;
//#    struct
//#    {
//#      u_short s_w1,s_w2;
//#    }
//#    S_un_w;
//#    u_long S_addr;
//#  }
//#  S_un;
//#} IN_ADDR, *PIN_ADDR, FAR *LPIN_ADDR;
//###RAPPEL_Structures_IPv4_AF_INET_SOCKETS####//


/************######################################**************************************/
//Fenetre Graphique WINDOWS
/*#include <SDL/SDL.h>

void pause()
{
    int continuer = 1;
    SDL_Event event;

    while (continuer)
    {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                continuer = 0;
        }
    }
}*/

//Fenetre Graphique LINUX
//le tuto et les lib http://www.gtk.org/download/win32_tutorial.php
//#include <gtk/gtk.h>



/************######################################**************************************/


static void showSocketErrorSystMsg (char const *csch_text) //La modif apres car complex pr rien
{
       perror (csch_text);
}

int clientTalking(int socket)
{
    printf ("\n\nCLIENT:>");
    int i_error = 0;
    int transmissionFinished = 0;


    while(transmissionFinished != 1)
    {
        unsigned char data[128];

        fgets (data, sizeof data, stdin);

        if (data[0] == 27) //ESC en code ascii
        {
            transmissionFinished = 1;
            printf ("\n\nCLIENT TCP: Fin de transmission - deconnexion en cours...\n\n");
        }

        int i_faillure;
        i_faillure=send (socket, data, strlen (data), 0);
        if(i_faillure>=0)
        {
             int newFileDescriptorSock = recv (socket, data, (int) sizeof data - 1, 0);

             if (newFileDescriptorSock != -1)
             {
                 size_t nb_rec = (size_t) newFileDescriptorSock;

                 data[nb_rec] = 0; //il faut marquer la fin

                 printf ("SERVEUR: %s\n",data);
             }
             else
             {
                 perror ("CLIENT TCP: erreur reception\n\n");
                 i_error = 1;
                 transmissionFinished = 1;
             }
        }
        else
        {
            perror ("CLIENT TCP: erreur transmission\n\n");
            i_error = 1;
            transmissionFinished = 1;
        }
    }
    return i_error;
}

static int lunchAppSocket (int mode, char* presenter)
{
   //Memo : TCP ou UDP sont 2 types de socket : SOCK_STREAM et SOCK_DGRAM.
   //En mode AF_INET, ce sont les numéros de port qui donnent le point de rendez-vous

   int i_error = 0;
   int i_socket_success=0;

   printf("%s TCP: Ouverture d'une socket %s en mode TCP/IP en cours...\n\n",presenter,presenter);

   //AF_INET-> IPV4, SOCK_STREAM->TCP
   int mySocket = socket (AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP. Le 0 -> pas de protocoles classiques. Ex: traitement paquets ICMP; IPPROTO_ICMP ->remplace 0

   if (mySocket > 0)
   {
        printf ("%s TCP: La socket %s %d est ouverte en mode TCP/IP. \n\n",presenter,presenter, mySocket);

        /***************************************************************************/
        if(mode== 0)//serveur
        {
            i_socket_success = serverBindingForListening (mySocket);

            if (i_socket_success != -1)
            {
                i_error=serverListening();
            }
            else
            {
                printf ("SERVEUR TCP: Impossible d'ecouter via la socket SERVEUR %d\n\n", mySocket);
                i_error=1;
            }
        }
        if(mode == 1) //client
        {
            i_socket_success=connection(mySocket,IP);
            if (i_socket_success != -1)
            {
                i_error=clientTalking(mySocket);
            }
            else
            {
                printf ("CLIENT TCP: Impossible de connecter le CLIENT (socket %d)\n\n", mySocket);
                i_error=1;
            }
        }
        /****************************************************************************/

        int i_tempo=0;
        i_tempo = closingSocket(&mySocket,presenter);

        if(i_error == 0)
            i_error ==i_tempo;
   }
   else
   {
      //showSocketErrorSystMsg ("%s TCP: Erreur ouverture socket",presenter);
      i_error = 1;
   }
   return i_error;
}

int closingSocket(int *Socket, char* name) //pour ne pas travailler av une copie mais av le vrai socket
{
    int i_error = 0;
    int i_errorSocket = 0;

    printf ("%s TCP: Fermeture de la socket '%s' %d en cours...\n\n",name, name, *Socket);

    i_errorSocket = close(*Socket);

    *Socket = -1;

    if (i_errorSocket != 0)
    {
       //showSocketErrorSystMsg ("%s TCP: Erreur fermeture socket\n\n",name);
       i_error = 1;
    }
    else
    {
       printf ("%s TCP: Fermeture de socket reussie\n\n",name);
    }
    return i_error;
}

int serverBindingForListening (int sock)
{
    int sock_err;

    SERVER_SOCKADRR_IN serveur ={0}; //pr assigner le porte d'ecoute
    serveur.sin_addr.s_addr = htonl (INADDR_ANY);//adress ip automatique
    serveur.sin_family = AF_INET; //protocol IP
    serveur.sin_port = htons (SERVER_LISTEN_PORT);//port d'écoute
    sock_err = bind (sock, (SOCKADDR *) &serveur, sizeof serveur);

    if(sock_err < 0 )
       printf("SERVEUR TCP: Echec de liaison : %d\n\n");
    else
       printf("SERVEUR TCP: Mise en liaison reussie.\n\n");

    return sock_err;
}

int serverListening(int sock)
{
    int i_error=0;
    int sock_err = 0;

    sock_err = listen(sock, MAXCONNEXQUEUE); //deuxieme parametre: taille de file d'attente des connectants

    if (sock_err >= 0 )
    {
        printf ("SERVEUR TCP: Ecoute sur le port %d...\n\n", SERVER_LISTEN_PORT);
        /***********************************************************************/
        serverKernel(sock);
        /***********************************************************************/
    }
    else
    {
        printf ("SERVEUR TCP: tentative d'écoute sur le port %d échouee.\n\n", SERVER_LISTEN_PORT);
        i_error = 1;
    }
    return i_error;
}

int connection(int sock/*, char* IP*/)
{
    int sock_err;

    SERVER_SOCKADRR_IN infoOfConnex ={0};
    infoOfConnex.sin_addr.s_addr = inet_addr (IP);//adresse serveur
    infoOfConnex.sin_family = AF_INET; //protocol IP

    infoOfConnex.sin_port = htons (CLIENT_LISTEN_PORT);//port de connection du serveur
    sock_err = connect (sock, (SOCKADDR *) & infoOfConnex, sizeof infoOfConnex);

    printf ("CLIENT TCP: Connexion au serveur %s:%d en cours...\n\n",IP, CLIENT_LISTEN_PORT);

    if(sock_err < 0 )
       printf("CLIENT TCP: Echec de la connection a %s:%d.\n\n",inet_ntoa(infoOfConnex.sin_addr),htons (infoOfConnex.sin_port),sock);
    else
       printf("CLIENT TCP: Connection a %s:%d reussie. (Socket %d).\n\n",inet_ntoa(infoOfConnex.sin_addr),htons (infoOfConnex.sin_port),sock);

    return sock_err;
}


struct strc_Client
{
   SERVER_SOCKADRR_IN clientAddressInfo;
   int i_sizeOfReceive_StructAdress_in;
   int i_theSocket;
   pthread_t associatedThread;
   int i_error;
};

static void *treatmentClientFunction(void *arg)
{
    int total_byte_receive = 0;
    struct strc_Client *p_client= arg;

    if (p_client != NULL)
    {
        printf ("\n\nCLIENT([%s:%d]SOCKET %d):",inet_ntoa(p_client->clientAddressInfo.sin_addr),htons (p_client->clientAddressInfo.sin_port),p_client->i_theSocket);
        int transmissionFinished =0;
        while(transmissionFinished != 1)
        {
            unsigned char data[128];
            int newFileDescriptorSock = recv (p_client->i_theSocket, data, (int) sizeof data - 1, 0);

            if (newFileDescriptorSock != -1)
            {
                size_t nb_rec = (size_t) newFileDescriptorSock;

                data[nb_rec] = 0; //il faut marquer la fin

                total_byte_receive += nb_rec;

                printf ("%s",data);
                fflush (stdout); //pr forcer lecriture du tampon de sortie

                if(data[nb_rec - 1] == 10) //ENTER en code ascii
                {
                    printf ("\nCLIENT([%s:%d]SOCKET %d):",inet_ntoa(p_client->clientAddressInfo.sin_addr),htons (p_client->clientAddressInfo.sin_port),p_client->i_theSocket);
                }
                if(strcmp(data,"janvier") == 0) //ENTER en code ascii
                {
                    printf ("\n\nSERVEUR TCP: vous êtes du signe ");
                }
                if (data[0] == 27) //ESC en code ascii
                {
                    transmissionFinished = 1;
                    printf ("\n\nSERVEUR TCP: %u byte%s recu%s du client IP %s:%d.\n",(unsigned) total_byte_receive, total_byte_receive > 1 ? "s" : "",total_byte_receive > 1 ? "s" : "",
                            inet_ntoa(p_client->clientAddressInfo.sin_addr),htons (p_client->clientAddressInfo.sin_port)); //sur ecran serveur

                    printf ("\n\nSERVEUR TCP: Fin de transmission - deconnexion en cours...\n\n");
                }
                else
                {
                    char const *cch_answer = "\n\nSERVEUR TCP: reception ok\n"; //sur l'ecran client
                    send (p_client->i_theSocket , cch_answer, strlen (cch_answer), 0);
                }
            }
            else
            {
                perror ("SERVEUR TCP: erreur reception\n\n");
                p_client->i_error = 1;
                transmissionFinished = 1;
            }
        }
        closingSocket(&p_client->i_theSocket,"CLIENT");
        p_client->i_theSocket=-1;

        free (p_client);
        p_client = NULL;
    }
    pthread_exit(NULL);
}

int serverKernel(int socketServer)
{
    int i_error=0;
    char *ch_msg;

    while(1)
    {
        printf("SERVEUR TCP: En attente d'une connexions...\n\n");

        struct strc_Client *p_client = malloc (sizeof *p_client);
        if (p_client != NULL)
        {
            p_client->i_sizeOfReceive_StructAdress_in = (int)sizeof(p_client->clientAddressInfo);
            p_client->i_theSocket = accept(socketServer,(struct SOCKADDR*)&p_client->clientAddressInfo,&p_client->i_sizeOfReceive_StructAdress_in);

            if (p_client->i_theSocket > 0)
            {
                printf("SERVEUR TCP: Connexion accepte\n\n");

                ch_msg = "SERVEUR TCP: [4AL - Andy - Rayane - Romain](test client)\n\n\n"
                         "SERVEUR TCP: Bienvenue sur le serveur PROGRAMMATION SYSTEME ET RESEAU 2014.\n\n";

                send(p_client->i_theSocket , ch_msg , strlen(ch_msg) , 0);

                fprintf(stdout, "SERVEUR TCP: Client connecte via l'adresse IP : %s:%d (socket %d)\n\n",inet_ntoa(p_client->clientAddressInfo.sin_addr),htons (p_client->clientAddressInfo.sin_port),p_client->i_theSocket);

                /***Thread de gestion du traitement serveur***/
                pthread_create (&p_client->associatedThread, NULL, treatmentClientFunction, p_client);
                /***ZombieLand****/
                p_client = NULL;
            }
            else
            {
                #if defined (WIN32) || defined (_WIN32) //WINDOWS et sockets 'WINSOCK'
                printf("SERVEUR TCP: L'acceptation de la connexion a echouee avec le code d'erreur: %d\n\n" , WSAGetLastError());
                #else //Linux et sockets 'BSD'
                printf("SERVEUR TCP:  L'acceptation de la connexion a echouee\n\n");
                #endif
                i_error = 1;
                //break;
            }
        }
        else
        {
            printf("SERVEUR TCP: Allocation dynamique d'espace mémoire pour le client a rencontre une erreur.\n\n");
            i_error = 1;
        }
    }
    return i_error;
}


int main (int argc, char** argv)
{
    int mode = 1; //mode serveur = 0
    char SERVEUR[]="SERVEUR";
    char CLIENT[]="CLIENT";
    char* presenter;

    if(argc>=2)
    {
       if(strcmp(argv[1],"serveur")!=-1)
       {
            mode =0;
       }
       else if(strcmp(argv[1],"client")!=-1)
       {
            mode =1;
       }
       else
       {
           printf("\nSERVEUR/CLIENT TCP: Argument(s) non reconnu(s)!\n\n");
           return -1;
       }
    }

    if(mode==0)
    {
        //printf("\n\nTEST: [argc: %d]"
        //       "\n[argv[0]: %s]\n[argv[1]: %s]\n\n",argc,argv[0],argv[1]);

        presenter = &SERVEUR;
        printf("\n#############################################"
               "\n#             MODE SERVEUR ACTIVE           #"
               "\n#############################################"
               "\n\n");
    }
    else
    {
        presenter = &CLIENT;
        printf("\n#############################################"
               "\n#             MODE CLIENT ACTIVE            #"
               "\n#############################################"
               "\n\n");
    }


    int i_error =0;
    int i_return;

    #if defined (WIN32) || defined (_WIN32) //WINDOWS et sockets 'WINSOCK'

        WSADATA wsa_myWSA;
        printf("[4AL - Andy - Rayane - Romain]\nWINDOWS OS detected...\n\n\n");
        printf("%s TCP: Phases d'initialisation du Winsock2 en cours...\n\n",presenter);

        i_error = WSAStartup (MAKEWORD (2, 2), &wsa_myWSA);  //MAKEWORD: combine deux caractères dans un mot de 16 bits

        if (i_error == 0)
            printf("%s TCP: Winsock2 initialisee\n\n",presenter);
        else
            printf("%s TCP: winsock2 non initialisée. Code d'erreur : %d",presenter,WSAGetLastError());

    #else //Linux et sockets 'BSD'

        printf("[4AL - Andy - Rayane - Romain]\nLinux OS detected...\n\n\n");
        printf("%s TCP: Phases d'initialisation du Winsock2 non nécessaire\n"
               "             BSD Unix pret\n\n",presenter);
        i_error =0;

    #endif

    /************START***************/
    i_error = lunchAppSocket(mode,presenter);

    if (i_error != 0)
    {
        printf("%s TCP: une erreur est survenue.\n\n",presenter);
    }

    #if defined (WIN32) || defined (_WIN32) //WINDOWS

        WSACleanup ();

    #else //Linux

    #endif

//##############ClôtureDeProgramme#############################

   if (i_error != 0 )
   {
      i_return = EXIT_FAILURE;
      printf("\n%s TCP: Fin de programme avec erreurs - Nb erreur:%d\n\n",presenter,i_error);
   }
   else
   {
      i_return = EXIT_SUCCESS;
      printf ("\n%s TCP: Fin de programme - Nb erreur:%d\n\n",presenter,i_error);
   }
   return i_return;
}
