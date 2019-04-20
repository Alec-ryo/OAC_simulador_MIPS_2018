#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define MEM_SIZE 4096

//Cria variaveis global
int32_t mem[MEM_SIZE];
int32_t opcode, rs, rt, rd, shamnt, funct, k16; 		//constante de 26 bits, para instruções tipo J
uint32_t pc, ri, k26, hi, lo, ini, fim;							//constante de 26 bits, para instruções tipo J
uint32_t breg[32];
int n_instr, leitura, get_ini, get_fim;
char formato;

/*PC em numero de bytes*/
/*opcode*/

//funcoes
void dump_mem(uint32_t add, uint32_t size, char format);
int32_t lw(uint32_t address, int16_t kte);
int32_t lh(uint32_t address, int16_t kte);
uint32_t lhu(uint32_t address, int16_t kte);
int32_t lb(uint32_t address, int16_t kte);
uint32_t lbu(uint32_t address, int16_t kte);
void sw(uint32_t address, int16_t kte, int32_t dado);
void sh(uint32_t address, int16_t kte, int16_t dado);
void sb(uint32_t address, int16_t kte, int8_t dado);
void fetch();
void decode();
void execute();
void step();
void run();

enum OPCODES
{
	EXT = 0x00,
	LW = 0x23,
	LB = 0x20,
	LBU = 0x24,
	ADDiu = 0x09,
	LH = 0x21,
	LHU = 0x25,
	LUI = 0x0F,
	SW = 0x2B,
	SB = 0x28,
	SH = 0x29,
	BEQ = 0x04,
	BNE = 0x05,
	BLEZ = 0x06,
	BGTZ = 0x07,
	ADDI = 0x08,
	SLTI = 0x0A,
	SLTIU = 0x0B,
	ANDI = 0x0C,
	ORI = 0x0D,
	XORI = 0x0E,
	J = 0x02,
	JAL = 0x03
} opcodes;

enum FUNCT
{
	ADD = 0x20,
	SUB = 0x22,
	MULT = 0x18,
	DIV = 0x1A,
	AND = 0x24,
	OR = 0x25,
	XOR = 0x26,
	NOR = 0x27,
	SLT = 0x2A,
	JR = 0x08,
	SLL = 0x00,
	SRL = 0x02,
	SRA = 0x03,
	SYSCALL = 0x0C,
	MFHI = 0x10,
	MFLO = 0x12,
	ADDU = 0x21,
	SLTU = 0x2B
} functs;

enum REG
{
	zero = 0, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6,
	s7, t8, t9, k0, k1, gp, sp, fp, ra 
}reg;

int main(int argc, char const *argv[])
{
	FILE *ftext, *fdata;
	int i = 0;

	//Abre e carrega o data.bin	
	fdata = fopen("data.bin","rb");
	if (fdata!=NULL) 
	{
		printf("Sucesso na leitura do data.bin");
		i = 0;
        while(fread(&mem[i+2048], 4, 1, fdata) != 0)
        {
            i++;
        }
	} else {
		printf("Falha no data.bin.\n");
		return 0;
	}
	fclose(fdata);

	//Abre e carrega o text.bin na memoria mem[i]
	ftext = fopen("text.bin","rb");
	if (ftext!=NULL) 
	{
		printf("Sucesso na leitura do text.bin\n\n");
		i = 0;
        while(fread(&mem[i], 4, 1, ftext) != 0)
        {
            i++;
        }	
		n_instr = i;
	} else {
		printf("Falha no .text.\n\n");
		return 0;
	}

	fclose(ftext);

	//seta os registadores para os valores padr�o
    pc = 0;
    hi = 0;
    lo = 0;
    for(i = 0; i<32; i++)
    {
        breg[i] = 0x00000000;
    }
    breg[28] = 0x1800; //$gp
    breg[29] = 0x3ffc; //$sp

	printf("\nPrint dos primeiros 7 registradores: \n");
	dump_mem(0,28,'h');
	i = 0;
	
	printf("Digite: ");
	printf("\n1.Para rodar o passo a passo dos registradores: step()");
	printf("\n2.Para rodar todo o programa: run()\n");
	scanf("%d", &leitura);

	if(leitura == 1){
		while(1){
			printf("\nNumero de passos: %d", i);
			printf("\nRegistrador ri: %x", ri);
			printf("\nDigite: ");
			dump_reg('h');
			step();
			getchar();
			i++;
		}
	} else if (leitura == 2){
		run();
	}
	return 0;
}

void fetch()
{
	ri = mem[pc>>2];
	pc += 4;
}

void decode()
{
	opcode = ri >> 26;
	rs = (ri << 6) >> 27;
	rt = (ri << 11) >> 27;
	rd = (ri << 16) >> 27;
	shamnt = (ri << 21) >> 27;
	funct = (ri << 26) >> 26;
	k16 = (ri << 16) >> 16;		//tipo I
	k26 = (ri << 6) >> 6;		//tipo J
}

void execute()
{
	uint32_t reg_aux;
	int64_t produto;
	int32_t q,r;
	char letra;
	int contador;
	int16_t aux_addi;
	uint32_t sinal;

	switch (opcode)
	{
	case EXT:
		switch (funct)
		{
		case ADD:
			breg[rd] = (int32_t)breg[rs] + (int32_t)breg[rt];
			break;

		case SUB:
			breg[rd] = (int32_t)breg[rs] - (int32_t)breg[rt];
			break;

		case MULT:
			produto = (int64_t)breg[rs] * (int64_t)breg[rt];
			//printf("rs = %x\n", rs);
			//printf("rt = %x\n", rt);
			//printf("rd = %x\n", rd);
			//printf("breg[rs] = %x\n", breg[rs]);
			//printf("breg[rt] = %x\n", breg[rt]);
			//printf("breg[rs] decimal = %d\n", breg[rs]);
			//printf("breg[rt] decimal = %d\n", breg[rt]);
			//printf("breg[rd] = %x\n", breg[rd]);
			//printf("breg[rd] = %x\n", produto);
			//printf("breg[rd] = %d\n", produto);
			lo = (uint32_t)(produto);
			hi = (uint32_t)(produto >> 32);
			break;

		case DIV:
            q = (int32_t)breg[rs] / (int32_t)breg[rt];
            r = (int32_t)breg[rs] % (int32_t)breg[rt];
            lo = q;
            hi = r;
			break;
		
		case AND:
			breg[rd] = breg[rs] & breg[rt];
			break;

		case OR:
			breg[rd] = breg[rs] | breg[rt];
			break;

		case XOR:
			breg[rd] = breg[rs] ^ breg[rt];
			break;

		case NOR:
			breg[rd] = ~(breg[rs] | breg[rt]);
			break;

		case SLT:
			if ((int)breg[rs] < (int)breg[rt])
        	{
            	breg[rd] = 1;
        	} else{
				breg[rd] = 0;
			} 
			break;

		case JR:
			pc = breg[31]; 	//Pois acrescenta +4 no fetch
			break;

		case SLL:
			breg[rd] = breg[rt] << shamnt;
			break;

		case SRL:
			breg[rd] = breg[rt] >> shamnt;
			break;

		case SRA:
			if(breg[rd] & 0x80000000 == 0x80000000){		//if negativo
				breg[rd] = breg[rt];
				for(int i = 0; i < shamnt; i++){
					breg[rd] = breg[rd] >> 1;
					breg[rd] = breg[rd] | 0x80000000;
				} 
			}else {
				breg[rd] = breg[rd] >> shamnt;			//se positivo
			}
			break;

		case SYSCALL:
		
			switch (breg[2])
			{
			case 1:
				printf("%d", breg[4]);
				break;

			case 4:

                contador = 0;
                reg_aux = breg[4];
				letra = lb(reg_aux, contador);
				printf("%c", letra);
                while (letra!='\0'){
					contador++;
					if (contador == 4){
						reg_aux+=4;
						contador = 0;
					}
					letra = lb(reg_aux, contador);
					printf("%c", letra);
                }
                break;

			case 10:
				printf("\n-- Program is finished running --\n");
				exit(0);
				break;
			}
			break;

		case MFHI:
			breg[rd] = hi;
			break;

		case MFLO:
			breg[rd] = lo;
			break;

		case ADDU:
			breg[rt] = breg[rs] + k16;
			break;

		case SLTU:
			if (breg[rs] < k16)
        	{
            	breg[rt] = 1;
        	} else{
				breg[rt] = 0;
			} 
			break;
		}
		
		break;

	case LW:	
		breg[rt] = lw(breg[rs], 0);	
		break;

	case LB:
		breg[rt] = lb(breg[rs], 0);
		break;

	case LBU:
		breg[rt] = lbu(breg[rs], 0);
		break;

	case ADDiu:
		sinal = k16 & 0x00008000;	
		if (sinal == 0x00008000){	//negativo
			breg[rt] = breg[rs] + (k16 | 0xffff0000);
		} else {					//positivo
			breg[rt] = breg[rs] + (k16 & 0x0000ffff);
		}
		break;

	case LH:
		breg[rt] = lh(breg[rs], 0);
		break;

	case LHU:
		breg[rt] = lhu(breg[rs], 0);
		break;

	case LUI:
		breg[rt] = (k16 << 16) & 0xffffffff;
		break;

	case SW:
		sw(breg[rs], k16, breg[rt]);
		break;

	case SB:
		sb(breg[rs], k16, breg[rt]);
		break;

	case SH:
		sh(breg[rs], k16, breg[rt]);
		break;

	case BEQ:
		if (breg[rs] == breg[rt]) {
			pc = pc + ( (int16_t)k16 * 4);
		}
		break;

	case BNE:
		if(breg[rs] != breg[rt])
		{
			pc += (k16 << 2);
		};
		break;

	case BLEZ: 
		if ((int)breg[rs] <= 0)
        {
            pc += (k16 << 2);
        }
		break;

	case BGTZ:
		if ((int)breg[rs] > 0)
        {
            pc += (k16 << 2);
        }
		break;

	case ADDI:
		aux_addi = k16 & 0x0000FFFF;
		breg[rt] = breg[rs] + aux_addi;				//soma de 16 bits
		break;

	case SLTI:
		if ((int32_t)breg[rs] < k16)
        {
            breg[rt] = 1;
        }
        else{
            breg[rt] = 0;
		}
		break;

	case SLTIU:
		if (breg[rs] < k16)
        {
            breg[rt] = 1;
        }
        else{
            breg[rt] = 0;
		}
		break;

	case ANDI:
		breg[rt] = k16 & breg[rs];
		break;

	case ORI:
		breg[rt] = k16 | breg[rs];
		break;

	case XORI:
		breg[rt] = k16 ^ breg[rs];
		break;

	case J:
		pc = (pc & 0xfc000000) | (k26 << 2);
		break;

	case JAL:
		breg[31] = pc; //salva o endereco de retorno em $ra
        pc = (pc & 0xfc000000) | (k26 << 2);
		break;

	default:
		printf("Instrucao Invalida!");
	}

}

void step()
{
	fetch();
	decode();
	execute();
}

void run()
{
    while(n_instr >= (pc>>2)){
		step();
		n_instr += 1;
	}

}

void dump_reg(char format)
{
    printf("\nRegistradores:\n\n");
    if(format == 'h')
    {
        printf("Reg:         Valores (hexadecimal):\n\n");
        printf("$zero:%#8x \n", breg[0]);
        printf("$at:  %#8x \n", breg[1]);
        printf("$v0:  %#8x \n", breg[2]);
        printf("$v1:  %#8x \n", breg[3]);
        printf("$a0:  %#8x \n", breg[4]);
        printf("$a1:  %#8x \n", breg[5]);
        printf("$a2:  %#8x \n", breg[6]);
        printf("$a3:  %#8x \n", breg[7]);
        printf("$t0:  %#8x \n", breg[8]);
        printf("$t1:  %#8x \n", breg[9]);
        printf("$t2:  %#8x \n", breg[10]);
        printf("$t3:  %#8x \n", breg[11]);
        printf("$t4:  %#8x \n", breg[12]);
        printf("$t5:  %#8x \n", breg[13]);
        printf("$t6:  %#8x \n", breg[14]);
        printf("$t7:  %#8x \n", breg[15]);
        printf("$s0:  %#8x \n", breg[16]);
        printf("$s1:  %#8x \n", breg[17]);
        printf("$s2:  %#8x \n", breg[18]);
        printf("$s3:  %#8x \n", breg[19]);
        printf("$s4:  %#8x \n", breg[20]);
        printf("$s5:  %#8x \n", breg[21]);
        printf("$s6:  %#8x \n", breg[22]);
        printf("$s7:  %#8x \n", breg[23]);
        printf("$t8:  %#8x \n", breg[24]);
        printf("$t9:  %#8x \n", breg[25]);
        printf("$k0:  %#8x \n", breg[26]);
        printf("$k1:  %#8x \n", breg[27]);
        printf("$gp:  %#8x \n", breg[28]);
        printf("$sp:  %#8x \n", breg[29]);
        printf("$fp:  %#8x \n", breg[30]);
        printf("$ra:  %#8x \n", breg[31]);
        printf("pc:   %#8x \n", pc);
        printf("hi:   %#8x \n", hi);
        printf("lo:   %#8x \n\n", lo);
    }
    else if(format == 'd')
    {
        printf("Reg:    Valores (decimal):\n\n");
        printf("$zero:\t%d\n", breg[0]);
        printf("$at:  \t%d\n", breg[1]);
        printf("$v0:  \t%d\n", breg[2]);
        printf("$v1:  \t%d\n", breg[3]);
        printf("$a0:  \t%d\n", breg[4]);
        printf("$a1:  \t%d\n", breg[5]);
        printf("$a2:  \t%d\n", breg[6]);
        printf("$a3:  \t%d\n", breg[7]);
        printf("$t0:  \t%d\n", breg[8]);
        printf("$t1:  \t%d\n", breg[9]);
        printf("$t2:  \t%d\n", breg[10]);
        printf("$t3:  \t%d\n", breg[11]);
        printf("$t4:  \t%d\n", breg[12]);
        printf("$t5:  \t%d\n", breg[13]);
        printf("$t6:  \t%d\n", breg[14]);
        printf("$t7:  \t%d\n", breg[15]);
        printf("$s0:  \t%d\n", breg[16]);
        printf("$s1:  \t%d\n", breg[17]);
        printf("$s2:  \t%d\n", breg[18]);
        printf("$s3:  \t%d\n", breg[19]);
        printf("$s4:  \t%d\n", breg[20]);
        printf("$s5:  \t%d\n", breg[21]);
        printf("$s6:  \t%d\n", breg[22]);
        printf("$s7:  \t%d\n", breg[23]);
        printf("$t8:  \t%d\n", breg[24]);
        printf("$t9:  \t%d\n", breg[25]);
        printf("$k0:  \t%d\n", breg[26]);
        printf("$k1:  \t%d\n", breg[27]);
        printf("$gp:  \t%d\n", breg[28]);
        printf("$sp:  \t%d\n", breg[29]);
        printf("$fp:  \t%d\n", breg[30]);
        printf("$ra:  \t%d\n", breg[31]);
        printf("pc:   \t%d\n", pc);
        printf("hi:   \t%d\n", hi);
        printf("lo:   \t%d\n\n", lo);
    }
    else
    {
        printf("Formato incorreto! \n");
    }
}


void dump_mem(uint32_t add, uint32_t size, char format)
{
	for (int i = add; i < (size / 4); i++)
	{
		if (format == 'h'){
			printf("mem[%d] = %x\n", i, mem[i]);
		} else if (format = 'd'){
			printf("mem[%d] = %d\n", i, mem[i]);
		} else {
			printf("Formato inválido");
			break;
		}
		
	}
}

int32_t lw(uint32_t address, int16_t kte)
{
	int resto = (address + kte) % 4;
	uint32_t endereco = (address + kte) / 4;
	uint32_t conteudo;
	if (resto == 0)
	{
		conteudo = mem[endereco];
		return conteudo;
	}
	else
	{
		printf("Endereco invalido 1\n");
		return 0;
	}
	printf("1");
}

int32_t lh(uint32_t address, int16_t kte)
{
	int resto = (address + kte) % 4;
	uint32_t endereco;
	int16_t valor;
	int32_t conteudo, sinal;
	endereco = (address + kte) / 4;
	conteudo = *(mem + endereco); //!!!!*(mem + endereco) é o conteudo!!!!
	sinal = conteudo & 0x00008000;

	if (resto == 0)
	{
		conteudo = (conteudo << 16) >> 16;
	}
	else if (resto == 2)
	{
		conteudo = conteudo >> 16;
	}
	else if (resto == 1 || resto == 3)
	{
		printf("Endereco invalido 2\n");
		return 0;
	}

	if (sinal == 0x00008000)
	{ //negativo
		conteudo = conteudo | 0xFFFF0000;
	}
	else
	{ //positivo
		conteudo = conteudo & 0x0000FFFF;
	}

	return conteudo;
}

uint32_t lhu(uint32_t address, int16_t kte)
{
	uint32_t endereco;
	int resto = (address + kte) % 4;
	int16_t valor;
	int32_t conteudo;
	endereco = (address + kte) / 4;
	conteudo = *(mem + endereco);
	if (resto == 0)
	{
		conteudo = ((conteudo << 16) >> 16) & 0x0000FFFF;
	}
	else if (resto == 2)
	{
		conteudo = (conteudo >> 16) & 0x0000FFFF;
	}
	else if (resto == 1 || resto == 3)
	{
		printf("Endereco invalido 3\n");
		return 0;
	}
	return conteudo;
}

int32_t lb(uint32_t address, int16_t kte)
{
	uint32_t endereco;
	int32_t conteudo, sinal;
	int resto = (address + kte) % 4;
	endereco = (address + kte) / 4;
	conteudo = mem[endereco];

	if (resto == 0)
	{
		conteudo = (conteudo << 24) >> 24;
	}
	else if (resto == 1)
	{
		conteudo = conteudo << 16;
		conteudo = conteudo >> 24;
	}
	else if (resto == 2)
	{
		conteudo = conteudo << 8;
		conteudo = conteudo >> 24;
	}
	else if (resto == 3)
	{
		conteudo = conteudo >> 24;
	}

	sinal = conteudo & 0x00000080;

	if (sinal == 0x00000080)
	{ //negativo
		conteudo = conteudo | 0xFFFFFF00;
	}
	else
	{ //positivo
		conteudo = conteudo & 0x000000FF;
	}

	return conteudo;
}

uint32_t lbu(uint32_t address, int16_t kte)
{
	uint32_t endereco;
	int32_t conteudo;
	int resto = (address + kte) % 4;
	endereco = (address + kte) / 4;
	conteudo = mem[endereco];

	if (resto == 0)
	{
		conteudo = (conteudo << 24) >> 24; 
		conteudo = conteudo & 0x000000FF;
	}
	else if (resto == 1)
	{
		conteudo = conteudo << 16;
		conteudo = conteudo >> 24;
		conteudo = conteudo & 0x000000FF;
	}
	else if (resto == 2)
	{
		conteudo = conteudo << 8;
		conteudo = conteudo >> 24;
		conteudo = conteudo & 0x000000FF;
	}
	else if (resto == 3)
	{
		conteudo = conteudo >> 24;
		conteudo = conteudo & 0x000000FF;
	}
	return conteudo;
}

void sw(uint32_t address, int16_t kte, int32_t dado)
{
	uint32_t endereco = (address + kte) / 4;
	mem[endereco] = dado;
}

void sh(uint32_t address, int16_t kte, int16_t dado)
{
	uint32_t endereco;
	int32_t valor = dado, conteudo;
	int resto = (address + kte) % 4;
	endereco = (address + kte) / 4;
	if (resto == 0)
	{
		conteudo = mem[endereco] & 0xffff0000;
		mem[endereco] = conteudo | (valor & 0x0000ffff);
	}
	else if (resto == 2)
	{
		conteudo = mem[endereco] & 0x0000FFFF;
		valor = valor << 16;
		mem[endereco] = conteudo | (valor & 0xffff0000);
	}
	else
	{
		printf("Endereço invalido");
	}
}

void sb(uint32_t address, int16_t kte, int8_t dado)
{
	uint32_t endereco;
	int32_t valor = dado, conteudo;
	int resto = (address + kte) % 4;
	endereco = (address + kte) / 4;
	if (resto == 0)
	{
		conteudo = mem[endereco] & 0xffffff00;
		mem[endereco] = conteudo | (valor & 0x000000ff);
	}
	else if (resto == 1)
	{
		conteudo = mem[endereco] & 0xffff00ff;
		valor = valor << 8;
		mem[endereco] = conteudo | (valor & 0x0000ff00);
	}
	else if (resto == 2)
	{
		conteudo = mem[endereco] & 0xff00ffff;
		valor = valor << 16;
		mem[endereco] = conteudo | (valor & 0x00ff0000);
	}
	else if (resto == 3)
	{
		conteudo = mem[endereco] & 0x00ffffff;
		valor = valor << 24;
		mem[endereco] = conteudo | (valor & 0xff000000);
	}
}
