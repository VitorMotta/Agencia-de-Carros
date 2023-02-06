/*
Vitor Vicente da Motta
Trabalho = Agencia de Carros
Banco de Dados
*/

#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>

//Opção 4 (Qual cliente efetuou o maior numero de compra/troca de carro no ultimo ano)
	void quem_trocou(){
	MYSQL *conn = mysql_init(NULL);
	MYSQL_RES *res1;
	MYSQL_RES *res2;
	MYSQL_ROW row1;
	MYSQL_ROW row2;
	char datastr1[9], datastr2[9];
	int i=0, j=0, datanum1, datanum2;
	
	char *query = (char*)malloc(800*sizeof(char));
	if(mysql_real_connect(conn, "localhost", "root", "Password123#@!", "agencia_carros", 0, NULL, 0)){
		
		//Pesquisa retorna informações dos clientes que realizaram troca de carro mais de uma vez.
		query = "select nome, CPF, data from Clientes natural join(select CPF_cliente as CPF, data from Historico where operacao = 'troca') as t1;";
		mysql_query(conn,query);
		res1 = mysql_store_result(conn);
		
		row1 = mysql_fetch_row(res1);
		row2 = mysql_fetch_row(res1);

		printf("nome_cliente\n-------------\n");

		//Loop para comparar linha por linha a data de uma troca com a data da proxima troca de cada cliente.
		while(1){
			//Compara se a linha atual corresponde ao mesmo cliente da linha anterior
			if(!strcmp(row1[0], row2[0])){
				i = 0;
				j = 0;
				
				while(row1[2][i] != '\0'){
					if(row1[2][i] >= '0' && row1[2][i] <= '9'){
						datastr1[j] = row1[2][i];
						j++;
					}
					i++;
				}
				datanum1 = atoi(datastr1);

				i = 0;
				j = 0;
				
				while(row2[2][i] != '\0'){
					if(row2[2][i] >= '0' && row2[2][i] <= '9'){
						datastr2[j] = row2[2][i];
						j++;
					}
					i++;
				}
				datanum2 = atoi(datastr2);

				//caso a diferença de data seja menor que 1 ano, imprime o nome do Cliente
				if(datanum1+10000 > datanum2){
					printf("%s\n-------------\n", row1[0]);
				}
			}

			row1 = row2;
			row2 = mysql_fetch_row(res1);
			if(row2 == NULL){
				break;
			}
		}
		char x[1];
		printf("\nDigite qualquer tecla para continuar\n");
		scanf("%s",x);
	}
	else{
		printf("Falha de conexão\n");
      	if(mysql_errno(conn)){
        	printf("Erro %d : %s\n", mysql_errno(conn), mysql_error(conn));
      	}
	}
}


//Executa a opção 14 (Apagar os registros dos clientes que não efetuam negócio (troca de carro ou manutenção) há mais de 3 anos)
void exclui_cliente(){
	MYSQL *conn = mysql_init(NULL);
	MYSQL_RES *res1;
	MYSQL_RES *res2;
	MYSQL_ROW row1;
	MYSQL_ROW row2;
	int i, j=1;
	char *CPF;

	char *query = (char*)malloc(800*sizeof(char));
	char *query2 = (char*)malloc(800*sizeof(char));

	//pesquisa que retorna informação dos clientes que efetuaram negocio a mais de 3 anos
	query = "select distinct CPF_cliente from Historico where data < date_sub(now(), interval 3 year);";

	if(mysql_real_connect(conn, "localhost", "root", "Password123#@!", "agencia_carros", 0, NULL, 0)){
		mysql_query(conn,query);
		res1 = mysql_store_result(conn);

        while((row1 = mysql_fetch_row(res1)) != NULL){
        	i = 1;

        	//pesquisa que retorna informação dos clientes que efetuaram negocio a menos de 3 anos
			query = "select distinct CPF_cliente from Historico where data > date_sub(now(), interval 3 year);";
			mysql_query(conn,query);
			res2 = mysql_store_result(conn);
        	
        	//compara se algum cliente que efetuou negocio a mais de 3 anos também efetuou alguma a menos de 3 anos
        	while((row2 = mysql_fetch_row(res2)) != NULL){
        		if(row1[0] == row2[0]){
        			i = 0;
        		}
        	}

        	//caso o(s) cliente(s) da primeira busca não apareçam na segunda, serão removidos 
        	if(i){
        		CPF = row1[0];
        		
        		sprintf(query2, "delete from Carros where dono = (select nome from Clientes where CPF = %s);", CPF);
				mysql_query(conn,query2);

				sprintf(query2, "delete from Historico where CPF_cliente = %s;", CPF);
				mysql_query(conn,query2);
				
				sprintf(query2, "delete from Clientes where CPF = %s;", CPF);
				mysql_query(conn,query2);

				printf("Cliente de CPF: %s foi removido com sucesso!\n", CPF);
				j = 0;
        	}
	    }

		//caso o(s) cliente(s) da primeira busca apareçam na segunda, nada será feito 
		if(j){
			printf("Nenhum cliente foi removido!\n");
		}
		else{
			mysql_free_result(res2);
		}
		mysql_free_result(res1);
		mysql_close(conn);
	}
	else{
		printf("Falha de conexão\n");
      	if(mysql_errno(conn)){
        	printf("Erro %d : %s\n", mysql_errno(conn), mysql_error(conn));
      	}
	}
	char x[1];
	printf("\nDigite qualquer tecla para continuar\n");

	scanf("%s",x);
}

//Executa a opção 13 (Demitir o funcionário com pior rendimento até um determinado momento. Os clientes do funcionário demitido passarão a pertencer ao penúltimo pior funcionário)
void demite_funcionario(char *data){
	MYSQL *conn = mysql_init(NULL);
	MYSQL_RES *res;
	MYSQL_ROW row;
	int i;
	char *cpf_pior, *cpf_segundo_pior;

	char *query = (char*)malloc(800*sizeof(char));

	//Busca retorna uma lista ordenada por quanto cada funcionario rendeu para a agencia em ordem crescente
	sprintf(query, "select CPF_funcionario as CPF, sum(lucro) as rendeu from Historico where data <= '%s' group by CPF_funcionario order by rendeu asc;", data);

	if(mysql_real_connect(conn, "localhost", "root", "Password123#@!", "agencia_carros", 0, NULL, 0)){
		mysql_query(conn,query);
		res = mysql_store_result(conn);//Recebe a consulta
		if(res){
	        row = mysql_fetch_row(res);
	        cpf_pior = row[0];
	        
	        row = mysql_fetch_row(res);

	        //caso haja apenas um funcionario, não será impossível removê-lo pois não há para quem passar seus clientes
	        if(row == NULL){
	        	printf("\nHá apenas um funcionario registrado. Impossível removê-lo!\n");
	        	char s[1];
				printf("\nDigite qualquer tecla para continuar\n");

				scanf("%s",s);
				return;
	        }
	        cpf_segundo_pior = row[0];

	        //Atribui aos clientes do pior funcionario o segundo pior funcionario. Em seguida, deleta o pior
	        sprintf(query, "update Clientes set CPF_funcionario = %s where CPF_funcionario = %s;", cpf_segundo_pior, cpf_pior);
	        mysql_query(conn,query);
	        sprintf(query, "update Historico set CPF_funcionario = %s where CPF_funcionario = %s;", cpf_segundo_pior, cpf_pior);
			mysql_query(conn,query);
			sprintf(query, "delete from Funcionarios where CPF = %s;", cpf_pior);
			mysql_query(conn,query);

			printf("\nFuncionario de CPF %s foi removido com sucesso!\n", cpf_pior);
		}
		mysql_free_result(res);
		mysql_close(conn);
	}
	else{
		printf("Falha de conexão\n");
      	if(mysql_errno(conn)){
        	printf("Erro %d : %s\n", mysql_errno(conn), mysql_error(conn));
      	}
	}
	char x[1];
	printf("\nDigite qualquer tecla para continuar\n");

	scanf("%s",x);
}


void computa(char *query){
	MYSQL *conn = mysql_init(NULL);
	MYSQL_RES *res;
	MYSQL_ROW row;
	MYSQL_FIELD *field;
	int i;

	if(mysql_real_connect(conn, "localhost", "root", "Password123#@!", "agencia_carros", 0, NULL, 0)){
		if(mysql_query(conn,query)){ 
	    	printf("Erro: %s\n",mysql_error(conn));
	   	}
	   	else{
			res = mysql_store_result(conn);//Recebe a consulta
			if(res){
				field = mysql_fetch_fields(res);
				printf("\n");
			}
			mysql_free_result(res);
		}
		mysql_close(conn);
	}
	else{
		printf("Falha de conexão\n");
      	if(mysql_errno(conn)){
        	printf("Erro %d : %s\n", mysql_errno(conn), mysql_error(conn));
      	}
	}
}

//Recebe a busca de forma generica, realiza e imprime o resultado
void imprime(char *query){
	MYSQL *conn = mysql_init(NULL);
	MYSQL_RES *res;
	MYSQL_ROW row;
	MYSQL_FIELD *field;
	int i;

	if(mysql_real_connect(conn, "localhost", "root", "Password123#@!", "agencia_carros", 0, NULL, 0)){
		if(mysql_query(conn,query)){ 
	    	printf("Erro: %s\n",mysql_error(conn));
	   	}
	   	else{
			res = mysql_store_result(conn);//Recebe a consulta
			if(res){
				field = mysql_fetch_fields(res);
				printf("\n");

				//imprime o nome de cada coluna
				for(i=0; i<mysql_num_fields(res); i++){
			        printf("(%s)", (field[i]).name);
			        if(mysql_num_fields(res)>1){
			        	printf("\t\t");
			        }
		        }
			    printf("\n");
			    for(i = 0; i < 90;i++){
		                printf("-");
		        }
			    printf("\n");
			    
			    //imprime todas as linhas do resultado da pesquisa
			    while((row = mysql_fetch_row(res)) != NULL){
			        for(i=0; i<mysql_num_fields(res); i++){
			            printf("%s\t\t", row[i]);
			        }
			        printf("\n");		
		            for(i = 0; i < 90;i++){
		                printf("-");
		            }
			        printf("\n");
			    }
			}
			mysql_free_result(res);
		}
		mysql_close(conn);
	}
	else{
		printf("Falha de conexão\n");
      	if(mysql_errno(conn)){
        	printf("Erro %d : %s\n", mysql_errno(conn), mysql_error(conn));
      	}
	}
	char x[1];
	printf("\nDigite qualquer tecla para continuar\n");

	scanf("%s",x);
}

void main(){
	MYSQL *conn = mysql_init(NULL);

	if(conn == NULL){
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	//Verifica a conexão com o banco de dados
	if(mysql_real_connect(conn, "localhost", "root", "Password123#@!", "agencia_carros", 0, NULL, 0)){
		printf("\t#######################\n");
		printf("\t#Conexão bem sucedida!#\n");
		printf("\t#######################\n");
		mysql_close(conn);
	}
	else{
		printf("Falha de conexão\n\n");
        printf("Erro %d : %s\n", mysql_errno(conn), mysql_error(conn));
	}

	int k,o;
	char *query = (char*)malloc(800*sizeof(char));
	char *query2 = (char*)malloc(800*sizeof(char));
	char *querymeta = (char*)malloc(800*sizeof(char));
	char *query_mais = (char*)malloc(800*sizeof(char));
	char *query_menos = (char*)malloc(800*sizeof(char));
	char *data1 = (char*)malloc(sizeof(char));
	char *data2 = (char*)malloc(sizeof(char));

	while(1){
		printf("\n\tMENU DE OPÇÕES\n");    	
    	printf("1 - Qual o modelo de carro mais vendido em um determinado intervalo de tempo\n");
  	 	printf("2 - Qual o carro menos vendido de todos os tempos\n");
		printf("3 - Quanto a agência lucrou com a venda/troca dentro de um determinado intervalo de tempo\n");
  	 	printf("4 - Qual o cliente efetuou o maior número de compra/troca de carro no último ano\n");
  	 	printf("5 - Quanto é a folha de pagamento mensal da agência\n");
		printf("6 - Quantos funcionários há em cada setor, e qual é o custo para manter esse setor\n");
		printf("7 - Obter os clientes que estão com revisão atrasadas (última revisão feita há mais de 1 ano atrás)\n");
  	 	printf("8 - Obter o gasto total (compra,troca, e/ou manutenções) por cliente\n");
  	 	printf("9 - Determinar quantos carros (por ano de fabricação), e o valor total destes, estão armazenados na agência\n");
  	 	printf("10 - Obter a lista de clientes que possuem carros antigos (ano de fabricação igual ou superior a 10 anos)\n");
  	 	printf("11 - Descobrir quais funcionários são mais e menos eficientes (em venda de carros)\n");
  	 	printf("12 - Adicionar bônus de 10%% ao salário dos funcionários que alcançarem a meta de R$1.000.000,00 em vendas\n");
  	 	printf("13 - Demitir o funcionário com pior rendimento até um determinado momento. Os clientes do funcionário demitido passarão a pertencer ao penúltimo pior funcionário\n");
  	 	printf("14 - Apagar os registros dos clientes que não efetuam negócio (troca de carro ou manutenção) há mais de 3 anos\n");
  	 	printf("15 - Sair\n");
  	 	printf("\nInsira o numero da opção: ");
  	 	
  	 	scanf("%d", &k);
  	 	printf("\n");

  	 	//Recebe a opção escolhida pelo usuario e faz a pesquisa correspondente
  	 	switch(k){
  	 		case 1:
   	 			printf("Entre com os limites do intervalo de tempo no formato AAAA-MM-DD\n");
	  	 		printf("Inicio: ");
	  	 		scanf("%s", data1);
	  			printf("Fim: ");
	   			scanf("%s", data2);
				sprintf(query2, "select modelo as modelo_mais_vendido, id from Carros natural join(select id_carro_novo as id, count(*) as quantidade from Historico where operacao = 'venda' and data >= '%s' and data <= '%s' group by id_carro_novo) as t1 order by quantidade desc limit 1;", data1, data2);  	 				
				imprime(query2);
				break;
  	 		case 2:
  	 			query = "select nome, modelo, ano from(select nome, modelo, ano, id, qtd from Carros natural join(select id_carro_novo as id, count(*) as qtd from Historico group by id_carro_novo order by qtd asc limit 1) as t1)as t2;";
  	 			imprime(query);
  	 			break;
  	 		case 3:
  	 			printf("Entre com os limites do intervalo de tempo no formato AAAA-MM-DD\n");
  	 			printf("Inicio: ");
  	 			scanf("%s", data1);
  	 			printf("Fim: ");
  	 			scanf("%s", data2);
				sprintf(query2, "select sum(lucro) as lucro_total from(select lucro from Historico where data > '%s' and data < '%s') as t1;", data1, data2);
				imprime(query2);
  	 			break;
  	 		case 4:
				quem_trocou();
  	 			break;
  	 		case 5:
				query = "select sum(salario) as total_folha_de_pagamento from Funcionarios;";
				imprime(query);
  	 			break;
  	 		case 6:
				query = "select nome, qtd_funcionarios, custo from Setores natural join (select cod_setor, count(cod_setor) as qtd_funcionarios, sum(salario) as custo from Funcionarios group by cod_setor) as t1;";
				imprime(query);
  	 			break;
  	 		case 7:
				query = "select dono as Cliente from Carros where dt_revisao < DATE_SUB(now(), INTERVAL 1 YEAR) and dono != 'agencia'";
				imprime(query);
  	 			break;
  	 		case 8:
				query = "select nome, gasto_total from (select nome, CPF, gasto_total from Clientes natural join (select CPF_cliente as CPF, sum(lucro) as gasto_total from Historico group by CPF_cliente)as t1)as t2;";
				imprime(query);
  	 			break;
  	 		case 9:
				query = "select ano as Ano_Fabricacao, sum(qtd_estoque) as qtd, sum(valor*qtd_estoque) as valor_total from Carros where dono = 'agencia' group by ano;";
				imprime(query);
  	 			break;
  	 		case 10:
				query = "select dono as nome_cliente from Carros where dono <> 'agencia' and ano <=((YEAR(now()))-10)";
				imprime(query);
  	 			break;
  	 		case 11:
				printf("1 - Mais eficiente\n2 - Menos eficiente\n\n");
				scanf("%d", &o);
					if (o == 1){
						query_mais = "select nome as funcionario_mais_eficiente from Funcionarios natural join (select CPF_funcionario as CPF, count(*) as vendas from Historico where operacao = 'venda' group by CPF_funcionario order by vendas desc limit 1) as t1;";
						imprime(query_mais);
					}else{
						query_menos = "select nome as funcionario_menos_eficiente from Funcionarios natural join (select CPF_funcionario as CPF, count(*) as vendas from Historico where operacao = 'venda' group by CPF_funcionario order by vendas asc limit 1) as t1;";
						imprime(query_menos);
					}	
  	 			break;
  	 		case 12:
	        	printf("Foi adicionado o bônus no salário do funcionário abaixo! \n");	
  	 			query = "update Funcionarios set salario = salario * 1.1 where CPF in (select CPF_funcionario from (select CPF_funcionario from Funcionarios, Historico where Funcionarios.CPF = Historico.CPF_funcionario group by CPF_funcionario having sum(lucro) >= 1000000.00) as t1);";
  	 			querymeta = "select nome from  Funcionarios, Historico where Funcionarios.CPF = Historico.CPF_funcionario group by CPF_funcionario having sum(lucro) >= 1000000.00;";
  	 			imprime(querymeta);
  	 			computa(query);
  	 			break;
  	 		case 13:
  	 			printf("Defina até qual momento no formato AAAA-MM-DD: ");
  	 			scanf("%s", data1);
  	 			demite_funcionario(data1);
  	 			break;
  	 		case 14:
  	 			exclui_cliente();
  	 			break;
  	 		case 15:
  	 			exit(0);
  	 			break;
  	 	}
	}
}
