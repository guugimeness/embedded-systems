void setup() {
  // Inicia a comunicação serial a uma taxa de 8928 bps (sincronizada com o 8051)
  Serial.begin(8928);
  
  // Aguarda a porta serial conectar (necessário em algumas placas como o Leonardo)
  while (!Serial) {
    ; 
  }
}

void loop() {
  // --- Envia a palavra "SistEmb-" letra por letra a cada 1 segundo ---

  Serial.print("S");
  delay(1000); // Aguarda 1 segundo
  
  Serial.print("i");
  delay(1000); // Aguarda 1 segundo
  
  Serial.print("s");
  delay(1000); // Aguarda 1 segundo
  
  Serial.print("t");
  delay(1000); // Aguarda 1 segundo
  
  Serial.print("E");
  delay(1000); // Aguarda 1 segundo
  
  Serial.print("m");
  delay(1000); // Aguarda 1 segundo
  
  Serial.print("b");
  delay(1000); // Aguarda 1 segundo
  
  Serial.print("-");
  delay(1000); // Aguarda 1 segundo
}