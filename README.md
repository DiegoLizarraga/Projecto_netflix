# Projecto_netflix

# Guía de Uso - Catálogo de Películas y Series

## Descripción General
Esta aplicación te permite gestionar un catálogo de películas y series con interfaz gráfica. Puedes filtrar contenido, calificar videos y navegar por portadas clickeables.

## Interfaz Principal

### Elementos de la Interfaz:
- **Menú desplegable**: Selecciona la opción que deseas ejecutar
- **Botón "Ejecutar"**: Ejecuta la opción seleccionada
- **Campo "Filtro"**: Ingresa términos de búsqueda (título o género)
- **"Cal. mín"**: Establece la calificación mínima para filtrar
- **Área de portadas**: Muestra las portadas de los videos (clickeables)
- **Área de resultados**: Muestra la información filtrada

## Opciones del Menú

### 1. Cargar archivo de datos
**¿Qué hace?**
- Abre un diálogo para seleccionar un archivo de texto (.txt)
- Permite cargar datos externos al catálogo

**Cómo usarla:**
1. Selecciona "1. Cargar archivo de datos" del menú
2. Presiona "Ejecutar"
3. Se abrirá un explorador de archivos
4. Selecciona el archivo .txt que contiene los datos
5. El resultado se mostrará en el área de resultados

**Nota:** Actualmente solo muestra el nombre del archivo cargado. Para implementación completa, el archivo debería contener datos en formato específico.

### 2. Mostrar videos por calificación/género
**¿Qué hace?**
- Filtra y muestra videos según género, título o calificación mínima
- Busca coincidencias en título y género

**Cómo usarla:**
1. Selecciona "2. Mostrar videos por calificación/género"
2. **Opcional**: Ingresa un término en el campo "Filtro" (ej: "Fantasía", "Star Wars")
3. **Opcional**: Establece una calificación mínima en "Cal. mín" (ej: 8.0)
4. Presiona "Ejecutar"
5. Los resultados aparecerán en el área de resultados

**Ejemplos de filtros:**
- Filtro: "Fantasía" → Muestra todos los videos de fantasía
- Cal. mín: 8.5 → Muestra videos con calificación ≥ 8.5
- Ambos → Muestra videos de fantasía con calificación ≥ 8.5

### 3. Mostrar episodios de serie
**¿Qué hace?**
- Filtra y muestra solo series que coincidan con los criterios
- Muestra información detallada de temporadas y episodios

**Cómo usarla:**
1. Selecciona "3. Mostrar episodios de serie"
2. **Opcional**: Ingresa el nombre de la serie en "Filtro" (ej: "Attack on Titan")
3. **Opcional**: Establece calificación mínima en "Cal. mín"
4. Presiona "Ejecutar"
5. Se mostrará información de series que coincidan

**Información mostrada:**
- Nombre de la serie
- Género
- Número de temporadas
- Total de episodios
- Director
- Calificación

### 4. Mostrar películas por calificación
**¿Qué hace?**
- Filtra y muestra solo películas con calificación igual o superior a la especificada

**Cómo usarla:**
1. Selecciona "4. Mostrar películas por calificación"
2. Establece la calificación mínima en "Cal. mín" (ej: 8.0)
3. Presiona "Ejecutar"
4. Se mostrarán todas las películas que cumplan el criterio

**Información mostrada:**
- Título de la película
- Género
- Duración en minutos
- Director
- Año
- Calificación

### 5. Calificar video
**¿Qué hace?**
- Permite asignar una nueva calificación a un video existente
- Actualiza la calificación promediándola con la anterior

**Cómo usarla:**
1. Selecciona "5. Calificar video"
2. **Obligatorio**: Ingresa el título (o parte del título) en el campo "Filtro"
3. Presiona "Ejecutar"
4. Aparecerá un diálogo pidiendo la calificación (1-10)
5. Ingresa la calificación y presiona OK
6. La nueva calificación se calculará como promedio de la anterior y la nueva

**Ejemplo:**
- Video actual: calificación 8.0
- Nueva calificación: 6.0
- Resultado: (8.0 + 6.0) / 2 = 7.0

### 0. Salir
**¿Qué hace?**
- Cierra completamente la aplicación

**Cómo usarla:**
1. Selecciona "0. Salir"
2. Presiona "Ejecutar"
3. La aplicación se cerrará

## Funciones Adicionales

### Portadas Clickeables
- **Qué son**: Las imágenes mostradas en la parte superior de la aplicación
- **Cómo usarlas**: Haz clic en cualquier portada para reproducir el video
- **Qué sucede**: 
  - Se abre el video con el reproductor predeterminado del sistema
  - Se muestra información del video
  - Si no existe el archivo, se muestra un mensaje de error

### Sistema de Archivos
La aplicación busca archivos en esta estructura:
```
./
├── portadas/
│   ├── default.jpg (imagen por defecto)
│   ├── la_princesa_mononoke.jpg
│   ├── el_viaje_de_chihiro.jpg
│   └── ...
└── videos/
    ├── peliculas/
    │   ├── la_princesa_mononoke.mp4
    │   └── ...
    └── series/
        ├── jujutsu_kaisen_s1e1.mp4
        └── ...
```

## Consejos de Uso

### Para Búsquedas Efectivas:
- Usa palabras clave parciales (ej: "Star" para encontrar todas las películas de Star Wars)
- Combina filtros de texto y calificación para resultados más precisos
- Los términos de búsqueda no distinguen mayúsculas/minúsculas

### Para Calificar Videos:
- Asegúrate de escribir correctamente parte del título
- Las calificaciones deben estar entre 1 y 10
- La nueva calificación se promedia con la existente

### Para Reproducir Videos:
- Asegúrate de tener los archivos de video en las rutas correctas
- Los videos se abren con el reproductor predeterminado del sistema
- Si no tienes los archivos, puedes usar la aplicación solo para gestionar información

## Datos Precargados

La aplicación viene con un catálogo predefinido que incluye:

**Películas:**
- Películas de Studio Ghibli (La princesa Mononoke, El viaje de Chihiro)
- Saga completa de Star Wars
- Look Back

**Series:**
- Anime populares (Attack on Titan, Jujutsu Kaisen, Demon Slayer)
- Series coreanas (Goblin)
- Contenido variado (Pokémon, Violet Evergarden)

Todos los videos tienen calificaciones, géneros y información detallada precargada para que puedas probar todas las funcionalidades inmediatamente.
