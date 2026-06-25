import time
import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# --- Configurações de Tempo ---
TEMPO_TOTAL_SEGUNDOS = 60  # Altere aqui para o tempo que você quiser coletar
tempo_inicio = time.time()  # Registra o momento exato em que o código começou

ser = serial.Serial('COM4', 115200, timeout=1)

# Cria a interface para as linhas dos gráficos
fig, ax = plt.subplots()
x_dados, y1_dados, y2_dados = [], [], []
linha1, = ax.plot([], [], 'b-', label='Altura')
linha2, = ax.plot([], [], 'r-', label='Erro')

# Indica o tamanho máximo e minimo de cada linha e o nome
ax.set_xlim(0, 10) # Eixo X agora representará segundos reais
ax.set_ylim(-15, 50)
ax.set_xlabel("Tempo (segundos)")
ax.set_ylabel("Valores")
ax.legend()

# Cria uma interface de texto para mostrar valores em tempo real
texto_valores = ax.text(
    0.02, 0.95, "", transform=ax.transAxes,
    fontsize=10, verticalalignment='top'
)

# Função para atualizar a todo momento o gráfico
def atualizar(frame):
    tempo_atual = time.time()
    tempo_decorrido = tempo_atual - tempo_inicio
    
    # 1. Verifica se o tempo limite foi atingido
    if tempo_decorrido >= TEMPO_TOTAL_SEGUNDOS:
        # Interrompe a animação desligando o timer do Matplotlib
        if hasattr(ani, 'event_source') and ani.event_source:
            ani.event_source.stop()
        print(f"Tempo limite de {TEMPO_TOTAL_SEGUNDOS}s atingido! Coleta finalizada.")
        return linha1, linha2

    if ser.in_waiting > 0:
        try:
            linha_serial = ser.readline().decode('utf-8').strip() #pega o código vindo do arduino
            if linha_serial:
                valores = linha_serial.split(',') #divide em 2 por conta da virgula
                if len(valores) == 2:
                    val1 = float(valores[0]) # vira a altura
                    val2 = float(valores[1]) # vira o erro

                    texto_valores.set_text(
                        f"Altura: {val1:.2f}\nErro: {val2:.2f}"
                    )
                    
                    print(f"Altura: {val1} | Erro: {val2}")

                    # Guardamos o tempo exato (em segundos) em que o dado chegou
                    x_dados.append(tempo_decorrido)
                    y1_dados.append(val1) #guarda o valor da altura e erro para atualizar no gráfico
                    y2_dados.append(val2)
                    
                    linha1.set_data(x_dados, y1_dados) #mostra na tela o valor atual
                    linha2.set_data(x_dados, y2_dados)
                    
                    # Ajusta o eixo X do zero até o tempo máximo definido
                    ax.set_xlim(0, TEMPO_TOTAL_SEGUNDOS)
                    
        except (ValueError, IndexError):
            pass #pula se tiver algum problema
            
    return linha1, linha2 #retorna a altura e erro

# Iniciamos a animação normalmente
ani = FuncAnimation(fig, atualizar, interval=20, cache_frame_data=False) #função que anima o gráfico para atualização em tempo real
plt.show() #mostra o gráfico
ser.close() #encerra conexão com arduino
