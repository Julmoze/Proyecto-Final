#include "huffman.h"

int Huffman::preparar_hojas(char *archivo)
{
    int j;
    for(j=0;j<256;++j){
        HOJAS[j].der=HOJAS[j].izq=HOJAS[j].arr=NULL;
        HOJAS[j].cuenta=0;
        HOJAS[j].karacter=j;
        HOJAS[j].codigo=NULL;
    }
    if ((f=fopen(archivo,"rb"))!=NULL){
        while ((j=fgetc(f))!=EOF){
            ++HOJAS[j].cuenta;
            ++NBYTES;
        }
        fclose(f);
    }
    else
    {
        return(1);
    }
    for(j=0;j<256;++j){
        if (HOJAS[j].cuenta!=0)
            ++NSIMB;
    }
    nsimb=NSIMB;
    return(0);
}

void Huffman::preparar_telar()
{
    int j;
    for(j=0;j<256;++j){
        TELAR[j]=&(HOJAS[j]);
    }
    return;
}

void Huffman::tejer()
{
    int menor=-1;     /* guarda indice */
    int segundo=-1;   /* guarda indice */
    int temporal;     /* guarda cuenta */
    int j;
    struct nodo *P;   /* nuevo nodo */

    if (nsimb==1) return;

    /* buscar menor valor */
    for(j=0;j<256;++j){
        if (TELAR[j]==NULL) continue;
        if (TELAR[j]->cuenta==0) continue;
        if (menor==-1){
            menor=j;
            temporal=TELAR[j]->cuenta;
        } else
        {
            if (TELAR[j]->cuenta<temporal){
                menor=j;
                temporal=TELAR[j]->cuenta;
            }
        }
    }

    /* buscar segundo menor */
    for(j=0;j<256;++j){
        if (TELAR[j]==NULL) continue;
        if (TELAR[j]->cuenta==0) continue;
        if (j==menor) continue;
        if (segundo==-1){
            segundo=j;
            temporal=TELAR[j]->cuenta;
        } else
        {
            if (TELAR[j]->cuenta<temporal){
                segundo=j;
                temporal=TELAR[j]->cuenta;
            }
        }
    }

    /* tejer un nuevo nodo */
    P=(struct nodo *)malloc(sizeof(struct nodo));
    TELAR[menor]->arr=P;
    TELAR[segundo]->arr=P;
    P->izq=TELAR[menor];
    P->der=TELAR[segundo];
    P->arr=NULL;
    TELAR[menor]->bit=0;
    TELAR[segundo]->bit=1;
    P->cuenta=TELAR[menor]->cuenta+TELAR[segundo]->cuenta;
    TELAR[menor]=NULL;
    TELAR[segundo]=P;
    --nsimb;

    /* sigue tejiendo hasta que sÃ³lo quede un nodo */
    tejer();
}

void Huffman::codificar()
{
    char pila[64];
    char tope;
    int j;
    char *w;
    struct nodo *P;
    for(j=0;j<256;++j){
        if (HOJAS[j].cuenta==0) continue;
        P=(struct nodo *)(&(HOJAS[j]));
        tope=0;
        while (P->arr!=NULL){
            pila[(int)tope]=P->bit;
            ++tope;
            P=P->arr;
        }
        HOJAS[j].nbits=tope;
        HOJAS[j].codigo=(char *)malloc((tope+1)*sizeof(char));
        w=HOJAS[j].codigo;
        --tope;
        while (tope>-1){
            *w=pila[(int)tope];
            --tope;
            ++w;
        }
        *w=2;
    }
    return;
}

void Huffman::debug()
{
    int j;
    char *w;
    int tam_comprimido=0;
    for(j=0;j<256;++j){
        if (HOJAS[j].cuenta==0) continue;
        tam_comprimido+=(HOJAS[j].cuenta*HOJAS[j].nbits);
        printf("%3d %6d ",j,HOJAS[j].cuenta);
        w=HOJAS[j].codigo;
        while (*w!=2){
            printf("%c",48+(*w));
            ++w;
        }
        printf("\n");
    }
    printf("NSIMB: %d\n",NSIMB);
    printf("NBYTES: %d\n",NBYTES);
    printf("TAMAÃ‘O COMPRIMIDO: %d\n",tam_comprimido/8+1);
    return;
}

int Huffman::escribe_cabecera(char *destino)
{
    int j,k;
    FILE *g;

    char *p=(char *)(&NBYTES);
    if ((g=fopen(destino,"wb"))==NULL) return(1);
    for(j=0;j<4;++j){
        fputc(*p,g);
        ++p;
    }

    p=(char *)(&NSIMB);
    fputc(*p,g);

    for(j=0;j<256;++j){
        if (HOJAS[j].cuenta==0) continue;
        fputc(j,g);
        p=(char *)(&(HOJAS[j].cuenta));
        for(k=0;k<4;++k){
            fputc(*p,g);
            ++p;
        }
    }
    fclose(g);
    return(0);
}

int Huffman::comprimir(char *origen, char *destino)
{
    unsigned char d=0;
    int x;
    char nbit=0;
    char *p;

    if ((f=fopen(origen,"rb"))==NULL) return(1);
    if ((g=fopen(destino,"ab"))==NULL) return(2); /* ya esta la cabecera */

    while ((x=fgetc(f))!=EOF){
        p=HOJAS[x].codigo;
        while (*p!=2){
            if (nbit==8){
                nbit=0;
                fputc(d,g);
                d=0;
            } else
            {
                if (*p==1){
                    d|=(1<<nbit);
                }
                ++nbit;
                ++p;
            }
        }
    }
    fputc(d,g);
    fclose(f);
    fclose(g);
    return(0);
}

int Huffman::descomprimir(char *origen, char *destino)
{
    char *p;
    int j,k,n,m;
    unsigned char x,nbit;
    struct nodo *P,*Q;

    if ((g=fopen(origen,"rb"))==NULL) return(1);
    if ((f=fopen(destino,"wb"))==NULL) return(2);

    /* leer NBYTES */
    p=(char *)(&n);
    for(j=0;j<4;++j){
        *p=(unsigned char)fgetc(g);
        ++p;
    }
    NBYTES=n;

    /* leer NSIMB */
    NSIMB=nsimb=fgetc(g);

    /* preparar las hojas */
    for(j=0;j<256;++j){
        HOJAS[j].cuenta=0;
        HOJAS[j].izq=HOJAS[j].der=HOJAS[j].arr=NULL;
        HOJAS[j].karacter=j;
    }
    for(j=0;j<NSIMB;++j){
        n=fgetc(g);
        p=(char *)(&m);
        for(k=0;k<4;++k){
            *p=(unsigned char)fgetc(g);
            ++p;
        }
        HOJAS[n].cuenta=m;
    }

    /* construyo el Ã¡rbol */
    preparar_telar();
    tejer();

    /* apunto a la raÃ­z del Ã¡rbol */
    j=0;
    while (HOJAS[j].cuenta==0) ++j;
    P=(struct nodo *)(&(HOJAS[j]));
    while (P->arr!=NULL) P=P->arr;

    /* ahora ya se puede descomprimir */
    j=0;
    x=fgetc(g);
    nbit=0;
    Q=P;
    while(j<NBYTES){
        if (Q->izq==NULL){
            fputc(Q->karacter,f);
            Q=P;
            ++j;
        } else
        if (nbit==8){
            x=fgetc(g);
            nbit=0;
        } else
        {
            if (x&(1<<nbit)){
                Q=Q->der;
            }
            else
            {
                Q=Q->izq;
            }
            ++nbit;
        }
    }
    fclose(f);
    fclose(g);
    return(0);
}

char Huffman::compressionFlow( std::string addressInput )
{
    char ext = addressInput.length() - 3;

    if( tolower(addressInput[ext]) == 'h' && tolower(addressInput[ext+1]) == 'u' && tolower(addressInput[ext+2]) == 'f'  )
    {
        return 'D';
    }

    return 'C';
}

std::string Huffman::addressInputToOutput( std::string addressInput )
{
    int ext = addressInput.length() - 3;

    char addressOutputTemp [1024];
    strcpy( addressOutputTemp, addressInput.c_str() );
    std::string addressOutput = addressOutputTemp;

    if( addressInput[ext] == 'h'  )
    {
        addressOutput[ext] = 't';
        addressOutput[ext+1] = 'x';
        addressOutput[ext+2] = 't';
    }else
    {
        addressOutput[ext] = 'h';
        addressOutput[ext+1] = 'u';
        addressOutput[ext+2] = 'f';
    }
    return addressOutput;
}

void Huffman::start( std::string addressInput )
{
    std::string addressOutput = addressInput;
    addressOutput =  addressInputToOutput ( addressInput );
    char convAddress = compressionFlow( addressInput );

    char addressInputTemp [1024];
    strcpy( addressInputTemp, addressInput.c_str() );
    char addressOutputTemp [1024];
    strcpy( addressOutputTemp, addressOutput.c_str() );
    char convAddressTemp [1024];
    convAddressTemp[0]=convAddress;
    char* argv[4];
    argv[1] = convAddressTemp;
    argv[2] = addressInputTemp;
    argv[3] = addressOutputTemp;

    //std::cout << argv[1] << "\n" << argv[2] << "\n" << argv[3] << "\n";

    convert(4,argv);
}
void Huffman::convert(int argc, char *argv[])
{
    if (argc<2) return;
    if (*(argv[1])=='C'){ /* comprimir */
        if (argc!=4) return;
        if (preparar_hojas(argv[2])){
            printf("error abriendo archivo\n");
            return;
        }
        preparar_telar();
        tejer();
        codificar();
        if (escribe_cabecera(argv[3])){
            printf("error abriendo archivo\n");
            return;
        }
        if (comprimir(argv[2],argv[3])){
            printf("error abriendo archivo\n");
            return;
        }
    }
    else
    if (*(argv[1])=='D'){ /* descomprimir */
        if (argc!=4) return;
    if (descomprimir(argv[2],argv[3])){
        printf("error abriendo archivo\n");
        return;
    }
    }
    else
    if (*(argv[1])=='I'){ /* info */
        if (argc!=3) return;
    if (preparar_hojas(argv[2])){
        printf("error abriendo archivo\n");
        return;
    }
    preparar_telar();
    tejer();
    codificar();
    debug();
    }
    return;
}
