#include <Arduino_FreeRTOS.h>

void TaskPrint(void *pvParameters);
void TaskCalc1(void *pvParameters);
void TaskCalc2(void *pvParameters);
void TaskCalc3(void *pvParameters);

// Contadores globais separados evitam race conditions
volatile long total_pontos_1 = 0;
volatile long pontos_circulo_1 = 0;
volatile long total_pontos_2 = 0;
volatile long pontos_circulo_2 = 0;
volatile long total_pontos_3 = 0;
volatile long pontos_circulo_3 = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial) { ; }   // Espera inicialização da serial
    
    Serial.println("Iniciando Calculo de PI - Modo: 4 Tarefas");
    randomSeed(analogRead(0));

    // Criação das duas tarefas:
    //    Cálculo com prioridade mais alta (2)
    xTaskCreate(TaskCalc1, "Calc1", 128, NULL, 2, NULL);
    xTaskCreate(TaskCalc2, "Calc2", 128, NULL, 2, NULL);
    xTaskCreate(TaskCalc3, "Calc3", 128, NULL, 2, NULL);

    //    Print com prioridade mais baixa (1)
    xTaskCreate(TaskPrint, "Print", 128, NULL, 1, NULL);
}

void loop() {
  // A função loop permanece vazia
}

// Executa uma iteração de Monte Carlo
void executa_monte_carlo(volatile long *total, volatile long *circulo) {
  float x = (float)random(1001) / 1000.0;
  float y = (float)random(1001) / 1000.0;
  
  if ((x * x + y * y) <= 1.0) {
    (*circulo)++;
  }
  (*total)++;
  
  // Cedência de controlo
  vTaskDelay(1); 
}

// Tarefas de Cálculo
void TaskCalc1(void *pvParameters) {
  (void) pvParameters;
  for (;;) { executa_monte_carlo(&total_pontos_1, &pontos_circulo_1); }
}

void TaskCalc2(void *pvParameters) {
  (void) pvParameters;
  for (;;) { executa_monte_carlo(&total_pontos_2, &pontos_circulo_2); }
}

void TaskCalc3(void *pvParameters) {
  (void) pvParameters;
  for (;;) { executa_monte_carlo(&total_pontos_3, &pontos_circulo_3); }
}

// Tarefa de Print
void TaskPrint(void *pvParameters) {
  (void) pvParameters;
  long total_anterior = 0;
  unsigned long tempo_inicio = millis();
  bool convergiu = false;
  
  for (;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    long total_atual = total_pontos_1 + total_pontos_2 + total_pontos_3;
    long circulo_atual = pontos_circulo_1 + pontos_circulo_2 + pontos_circulo_3;
    unsigned long tempo_atual = millis();
    
    if (total_atual > 0) {
      float pi_estimado = 4.0 * ((float)circulo_atual / (float)total_atual);
      long vazao = total_atual - total_anterior;
      total_anterior = total_atual;
      
      float erro = abs(pi_estimado - 3.14159265);
      unsigned long tempo_decorrido = (tempo_atual - tempo_inicio) / 1000;
      
      Serial.print("Tempo: ");
      Serial.print(tempo_decorrido);
      Serial.print("s | Total: ");
      Serial.print(total_atual);
      Serial.print(" | Vazao: ");
      Serial.print(vazao);
      Serial.print(" it/s | Pi: ");
      Serial.print(pi_estimado, 6);
      Serial.print(" | Erro: ");
      Serial.println(erro, 6);
      
      // Critério de convergência: erro menor que 0.005 e pelo menos 1000 iterações
      if (!convergiu && erro < 0.005 && total_atual > 1000) {
        convergiu = true;
        Serial.print(">>> CONVERGIU (Erro < 0.005) em: ");
        Serial.print(tempo_decorrido);
        Serial.println(" segundos! <<<");
        Serial.println("Execucao finalizada. Parando o sistema...");
        Serial.flush();
        
        // Suspende o scheduler do FreeRTOS e para a execução
        vTaskSuspendAll();
        while (true) {
          // Loop infinito para parar tudo
        }
      }
    }
  }
}
