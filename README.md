# Projeto

## Jogo

O jogo escolhido foi o Tennis Legends, um jogo de ate 2 players onde o melhor tenista ganha.
link para o jogo: https://www.snokido.com/game/tennis-legends-2016

## Como ligar o controle

Ligue os controles pelo switch localizado no canto esquerdo superior; se o LED comecar a piscar o controle esta ligado.
Para conectar os controles rode os comandos:

```sudo rfcomm connect /dev/rfcomm0 98:DA:60:08:7E:8F```

e

```sudo rfcomm connect /dev/rfcomm1 98:DA:60:08:83:BE```

(em diferentes terminais.)

Se os leds ficarem acesos o bluetooth foi conectado com sucesso.
Depois rode o python em outro terminal com:

```sudo python3 main.py```(apos dar 'cd python' na raiz do projeto)

## Como jogar

para jogar e simples; existem 4 movimentos principais:

A movimentcao: simplesmente mexe o personagem para a esquerda e para direita usando o joystick.

O smash: para fazer isso apenas gire o controle de cima para baixo,
esse movimento serve para cortadas e saque.

A batida: para fazer isso apenas gire o controle de de um lado para o outro,
esse movimento serve para bolas fundas e lobbys.

O salta: para isso apenas de um 'flick' no joystick para cima. Combinar o salto com o smash pode ser poderoso.

Ah, e lembre-se que poderes especiais podem aparecer! Tente acerta-los!