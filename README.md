# SaukOS
Primero hay que darle los permisos de ejecucion a tetris.sh y su carpeta de modulos con chmod +x
|| Deben dar los permisos al archivo ejecutable SaukOS y luego ejecutarlo con ./SaukOS
|| luego deben compilar el c
|| Luego dar los permisos al ejecutable que se generó al compilar el y finalmente ejecutan el mismo archivo


```git clone https://github.com/Slavitza/SaukOS.git```
```cd SaukOS```
```cd src```
```cd Tetris && chmod +x tetris.sh modules/*.sh```
```cd ..```
```gcc -o SaukOS simple-shell.c```
```chmod +x SaukOS```
```./SaukOS```

