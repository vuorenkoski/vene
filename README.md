# Kauko-ohjattava vene

Mikrokontrollerin koodi kauko-ohjattavalle veneelle.
- kummassakin päässä mikrokontrollerina ATMEGA-328, 16mhz
- kummassakin päässä radiona simppeli 433mhz lähetin/vastaanotin (DX.com Large power long range 433mhz wireless transceiver kit for arduino) 
- lähetystaajuus 1000 baudia
- yksi paketti 52bit, virheenkorjaus hammingkoodilla
- peräsin 6bit, moottori 1bit ja vilkkuvalot 1bit

Kytkennät, lähetin
- radio D11
- valot D10
- Moottori D12
- Peräsin A7

Kytkennät, vastaanotin
- radio D2
- valot D3,D4
- Peräsin D9
- Moottori D10


