/*
Client du Jeu du pendu
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


#define SERVICE_DEFAUT 33016
#define SERVEUR_DEFAUT "localhost"

#define BUFFER_SIZE 1024

#define DEBUG 1

int client_appli (char *serveur, int service);


			/*****************************************************************************/
				/*--------------- Programme Client -----------------------*/
			/*****************************************************************************/

main(int argc, char *argv[])
{

	char *serveur= SERVEUR_DEFAUT;
	int service= SERVICE_DEFAUT; /* no de port) */


	/* Permet de passer un nombre de parametre variable a l'executable */
	switch(argc)
	{
		case 1 :		
			  printf("\nserveur par defaut: %s\n",serveur);
			  printf("\nservice par defaut: %d\n",service);
			  break;
		case 2 :		
			  serveur=argv[1];
			  printf("service par defaut: %d\n",service);
			  break;
		case 3 :		
			  serveur=argv[1];
			  service=atoi(argv[2]);
			  break;
		
		default:
			  printf("Usage: \n> ./client ip port \n");
			  return EXIT_SUCCESS;
	}

	/* serveur est le nom (ou l'adresse IP) auquel le client va acceder */
	/* service le numero de port sur le serveur correspondant au  */
	/* service desire par le client */

	client_appli(serveur,service);

	return EXIT_SUCCESS;
}

int cree_socket_tcp_ip_client(int port, char* adresse_ip)
{
	int sock;
	struct sockaddr_in adresse;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Erreur socket\n");
		return -1;
	}

	memset(&adresse, 0, sizeof(struct sockaddr_in));
	adresse.sin_family = AF_INET;
	adresse.sin_port = htons(port);
	inet_aton(adresse_ip, &adresse.sin_addr);

	if (connect(sock, (struct sockaddr*) &adresse, sizeof(struct sockaddr_in)) < 0)
	{
		close(sock);
		fprintf(stderr, "Erreur connect\n");
		return -1;
	}

	return sock;
}

int client_appli (char *serveur,int service)
{
	int sock;

	char nomTube[BUFFER_SIZE];	
 	char lettre[BUFFER_SIZE];
	char choix[BUFFER_SIZE];
	char message[BUFFER_SIZE];
	char pseudo[BUFFER_SIZE];
    
	char result[BUFFER_SIZE];
       
	int continuer;
	int coupsRestants;	
	
	sock = cree_socket_tcp_ip_client(service,serveur);
	if (sock < 0)
	{
		return -1;
	}
    
	printf("\nEntrez un pseudo: ");
	scanf("%s", pseudo);

        if (write(sock, pseudo, strlen(pseudo)+1) < 0)
    	{
        	perror("write sock");
        	exit(EXIT_FAILURE);
        	return -1;
    	}
	
	printf("Connexion au serveur en cours.\n"); 

	if (read(sock, message, BUFFER_SIZE) < 0)
    	{
        	perror("read sock");
        	exit(EXIT_FAILURE);
        	return -1;
    	}
	
	printf("\nBienvenue dans le Jeu du Pendu %s !\n", message);
	
	printf("__________________________\n");
    	printf("                          \n");
    	printf("/q          - Quitter le serveur\n");
    	printf("__________________________\n\n");

	while (1)
	{	

		if(strcmp(lettre,"/q")==0)
		{

			
			printf("Vous quittez le Jeu.\n");
    			printf("__________________________________________________\n\n");

    			/*________________________________________________________*/
    			/*--------------------------------------------------------*/
    
    			printf("\nfin de la reception.\n");
    			close(sock);
    
    			printf("connexion avec le serveur fermee, fin du programme.\n");	
			break;	
		}	
	
		if (read(sock, result, BUFFER_SIZE) < 0)
    		{
        		perror("read sock");
        		exit(EXIT_FAILURE);
        		return -1;
    		}
    
		

    		if ((strstr(result, "Gagne") != NULL) || (strstr(result, "Perdu") != NULL))
    		{
        
			printf("%s\n", result);
			printf("\n\nVoulez vous continuer ? Oui(1) ou Non(0)\n");
			scanf("%d",&continuer);
			sprintf(choix,"%d",continuer);

			if(continuer==1)
			{

				if (write(sock, choix, strlen(choix)+1) < 0)
			    	{
			        	perror("write sock");
			        	exit(EXIT_FAILURE);
			        	return -1;
			    	}	

				if (read(sock, message, BUFFER_SIZE) < 0)
			    	{
			        	perror("read sock");
			        	exit(EXIT_FAILURE);
			        	return -1;
			    	}
    	
			}
		
			else
			{
				/*if (read(sock, message, BUFFER_SIZE) < 0)
			    	{
			        	perror("read sock");
			        	exit(EXIT_FAILURE);
			        	return -1;
			    	}*/

				printf("Fin de la partie\n");
				printf("Vous quittez le Jeu.\n");
	    			printf("__________________________________________________\n\n");

	    			/*________________________________________________________*/
	    			/*--------------------------------------------------------*/
    
	    			printf("\nfin de la reception.\n");
	    			close(sock);
	    
	    			printf("connexion avec le serveur fermee, fin du programme.\n");	
				break;	
			}
		}
	
	        printf("\nmot: %s\n", result);
    

	        printf("\nProposez une lettre : ");
	        scanf("%s", lettre);

		if (write(sock, lettre, strlen(lettre)+1) < 0)
		{
			perror("write sock");
			exit(EXIT_FAILURE);
			return -1;
		}	

		if (DEBUG == 1)
	        {
	            printf("Le message a envoyer est '%s'. \n", lettre);
	        }

	}
     
	//close(sock);
	return 0;
}


    
