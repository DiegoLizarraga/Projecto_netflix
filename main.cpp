#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Spinner.H>
#include <FL/fl_ask.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_File_Chooser.H>
#include <ctime>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <set>
#include <map>

// Declaración adelantada
class CatalogoApp;

// Función para convertir título a nombre de archivo
std::string tituloANombreArchivo(const std::string& titulo) {
    std::string nombre = titulo;
    std::string resultado;
    for (char c : nombre) {
        if (c >= 'A' && c <= 'Z') {
            resultado += (c + 32); // Minúscula
        } else if (c == ' ') {
            resultado += '_';
        } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
            resultado += c;
        }
    }
    return resultado;
}

// Clase base Video
class Video {
protected:
    std::string titulo;
    double calificacion;
    std::string genero;
    std::string director;
    int anio;
    std::string rutaPortada;
    std::string basePath;

public:
    // Sobrecarga de operadores
    Video& operator+=(double puntos) {
        calificacion = std::min(10.0, calificacion + puntos);
        return *this;
    }

    Video& operator-=(double puntos) {
        calificacion = std::max(0.0, calificacion - puntos);
        return *this;
    }

    Video(const std::string& t, double cal, const std::string& g, 
          const std::string& dir, int a)
        : titulo(t), calificacion(cal), genero(g), director(dir), 
          anio(a), basePath(".\\") {
        std::string nombreArchivo = tituloANombreArchivo(titulo);
        rutaPortada = basePath + "portadas\\" + nombreArchivo + ".jpg";
    }
        
    virtual ~Video() = default;
        
    // Función friend para operator<<
    friend std::ostream& operator<<(std::ostream& os, const Video& video) {
        os << video.getInfo();
        return os;
    }
    
    virtual std::string getTipo() const = 0;
    virtual std::string getInfo() const = 0;
    virtual std::string getRutaVideo() const = 0;
    
    std::string getTitulo() const { return titulo; }
    std::string getGenero() const { return genero; }
    std::string getRutaPortada() const { return rutaPortada; }
    double getCalificacion() const { return calificacion; }
    std::string getDirector() const { return director; }
    int getAnio() const { return anio; }
    
    void actualizarCalificacion(int nuevaCalificacion) {
        calificacion = (calificacion + nuevaCalificacion) / 2.0;
    }
    
    void setCalificacion(double cal) {
        calificacion = cal;
    }
    
    void setGenero(const std::string& g) {
        genero = g;
    }
    
    bool existePortada() const {
        std::ifstream file(rutaPortada);
        return file.good();
    }
    
    std::string getRutaPortadaODefault() const {
        if (existePortada()) {
            return rutaPortada;
        }
        return basePath + "portadas\\default.jpg";
    }

    // Sobrecarga de operadores de comparación
    bool operator<(const Video& other) const {
        return calificacion < other.calificacion;
    }

    bool operator>(const Video& other) const {
        return calificacion > other.calificacion;
    }

    bool operator==(const Video& other) const {
        return titulo == other.titulo;
    }

    bool operator!=(const Video& other) const {
        return !(*this == other);
    }
};

// Clase para manejar el historial de calificaciones
class HistorialManager {
private:
    std::string rutaHistorial;
    
    std::string obtenerFechaHora() {
        time_t now = time(0);
        char* dt = ctime(&now);
        std::string fecha(dt);
        fecha.pop_back(); // Remover el \n del final
        return fecha;
    }

public:
    HistorialManager(const std::string& ruta = "C:\\Users\\DiegoB\\Desktop\\Netflix_piraton\\historialDatos.txt") 
        : rutaHistorial(ruta) {}
    
    // Cargar calificaciones desde el archivo
    void cargarHistorial(std::vector<std::shared_ptr<Video>>& catalogo) {
        std::ifstream archivo(rutaHistorial);
        if (!archivo.is_open()) {
            // Si no existe el archivo, lo creamos con datos iniciales
            crearArchivoInicial();
            return;
        }
        
        std::string linea;
        while (std::getline(archivo, linea)) {
            if (linea.empty() || linea[0] == '#') continue;
            
            std::vector<std::string> partes = dividirCadena(linea, '|');
            if (partes.size() >= 3 && partes[0] == "CALIFICACION") {
                std::string titulo = partes[1];
                double calificacion = std::stod(partes[2]);
                
                // Buscar el video en el catálogo y actualizar su calificación
                for (auto& video : catalogo) {
                    if (video && video->getTitulo() == titulo) {
                        video->setCalificacion(calificacion);
                        break;
                    }
                }
            }
        }
        archivo.close();
    }
    
    // Guardar nueva calificación en el archivo
    void guardarCalificacion(const std::string& titulo, double nuevaCalificacion) {
        std::ofstream archivo(rutaHistorial, std::ios::app);
        if (archivo.is_open()) {
            archivo << "CALIFICACION|" << titulo << "|" << nuevaCalificacion 
                   << "|" << obtenerFechaHora() << std::endl;
            archivo.close();
        }
    }
    
    // Actualizar calificación existente en el archivo (reescribir todo el archivo)
    void actualizarHistorialCompleto(const std::vector<std::shared_ptr<Video>>& catalogo) {
        std::ofstream archivo(rutaHistorial);
        if (archivo.is_open()) {
            archivo << "# Historial de calificaciones - Netflix Piratón\n";
            archivo << "# Formato: CALIFICACION|Título del Video|Nueva Calificación|Fecha y Hora\n";
            archivo << "# Este archivo se actualiza automáticamente cuando calificas videos\n\n";
            archivo << "# Calificaciones actualizadas - " << obtenerFechaHora() << "\n";
            
            for (const auto& video : catalogo) {
                if (video) {
                    archivo << "CALIFICACION|" << video->getTitulo() << "|" 
                           << video->getCalificacion() << std::endl;
                }
            }
            archivo.close();
        }
    }

private:
    std::vector<std::string> dividirCadena(const std::string& cadena, char separador) {
        std::vector<std::string> resultado;
        std::stringstream ss(cadena);
        std::string item;
        
        while (std::getline(ss, item, separador)) {
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);
            resultado.push_back(item);
        }
        return resultado;
    }
    
    void crearArchivoInicial() {
        std::ofstream archivo(rutaHistorial);
        if (archivo.is_open()) {
            archivo << "# Historial de calificaciones - Netflix Piratón\n";
            archivo << "# Formato: CALIFICACION|Título del Video|Nueva Calificación|Fecha y Hora\n";
            archivo << "# Este archivo se actualiza automáticamente cuando calificas videos\n\n";
            archivo << "# Datos iniciales del catálogo (calificaciones base)\n";
            
            // Calificaciones iniciales
            archivo << "CALIFICACION|La princesa Mononoke|8.4\n";
            archivo << "CALIFICACION|El viaje de Chihiro|8.6\n";
            archivo << "CALIFICACION|Look Back|8.1\n";
            archivo << "CALIFICACION|Star Wars Episodio I La amenaza fantasma|6.5\n";
            archivo << "CALIFICACION|Star Wars Episodio II El ataque de los clones|6.5\n";
            archivo << "CALIFICACION|Star Wars Episodio III La venganza de los Sith|7.5\n";
            archivo << "CALIFICACION|Star Wars Episodio IV Una nueva esperanza|8.6\n";
            archivo << "CALIFICACION|Star Wars Episodio V El imperio contraataca|8.7\n";
            archivo << "CALIFICACION|Star Wars Episodio VI El retorno del Jedi|8.3\n";
            archivo << "CALIFICACION|Jujutsu Kaisen|8.7\n";
            archivo << "CALIFICACION|Pokemon|7.5\n";
            archivo << "CALIFICACION|Violet Evergarden|8.8\n";
            archivo << "CALIFICACION|Kimetsu no Yaiba|8.7\n";
            archivo << "CALIFICACION|Attack on Titan|9.0\n";
            archivo << "CALIFICACION|Blue Lock|8.3\n";
            archivo << "CALIFICACION|Star Wars The Clone Wars|8.4\n";
            archivo << "CALIFICACION|Ann|7.9\n";
            archivo << "CALIFICACION|Nadie nos va a extrañar|8.1\n";
            archivo << "CALIFICACION|Si la vida te da mandarinas|7.8\n";
            archivo << "CALIFICACION|Goblin|8.9\n";
            archivo << "CALIFICACION|Alien Stage|8.5\n";
            
            archivo.close();
        }
    }
};

// Clase derivada Película
class Pelicula : public Video {
private:
    int duracion;
    
public:
    Pelicula(const std::string& t, double cal, int d, const std::string& g, 
             const std::string& dir, int a)
        : Video(t, cal, g, dir, a), duracion(d) {}
    
    std::string getTipo() const override { return "Pelicula"; }
    
    std::string getInfo() const override {
        std::ostringstream oss;
        oss << "Película: " << titulo << " | Género: " << genero 
            << " | Duración: " << duracion << " min | Director: " << director
            << " | Año: " << anio << " | Calificación: " << std::fixed << std::setprecision(1) << calificacion;
        return oss.str();
    }
    
    std::string getRutaVideo() const override {
        std::string nombreArchivo = tituloANombreArchivo(titulo);
        return basePath + "videos\\peliculas\\" + nombreArchivo + ".mp4";
    }
    
    int getDuracion() const { return duracion; }
};

// Clase derivada Serie
class Serie : public Video {
private:
    int episodiosPorTemporada;
    int numTemporadas;
    int totalEpisodios;
    
public:
    Serie(const std::string& t, double cal, int ept, const std::string& g, 
          int nt, int te, const std::string& dir)
        : Video(t, cal, g, dir, 0), episodiosPorTemporada(ept), 
          numTemporadas(nt), totalEpisodios(te) {}
    
    std::string getTipo() const override { return "Serie"; }
    
    std::string getInfo() const override {
        std::ostringstream oss;
        oss << "Serie: " << titulo << " | Género: " << genero 
            << " | Temporadas: " << numTemporadas << " | Episodios: " << totalEpisodios
            << " | Director: " << director << " | Calificación: " << std::fixed << std::setprecision(1) << calificacion;
        return oss.str();
    }
    
    std::string getRutaVideo() const override {
        std::string nombreArchivo = tituloANombreArchivo(titulo);
        return basePath + "videos\\series\\" + nombreArchivo + "_s1e1.mp4";
    }
    
    int getEpisodiosPorTemporada() const { return episodiosPorTemporada; }
    int getNumTemporadas() const { return numTemporadas; }
    int getTotalEpisodios() const { return totalEpisodios; }
    
    std::string getEpisodiosInfo() const {
        std::ostringstream oss;
        int epNum = 1;
        for (int temp = 1; temp <= numTemporadas; temp++) {
            int epsEstaTemporada = std::min(episodiosPorTemporada, totalEpisodios - (epNum - 1));
            for (int ep = 1; ep <= epsEstaTemporada; ep++, epNum++) {
                oss << "T" << temp << "E" << ep << ": Episodio " << epNum << "\n";
            }
        }
        return oss.str();
    }
};

// Widget para portadas
class PortadaBox : public Fl_Box {
private:
    std::shared_ptr<Video> video;
    Fl_Image* imagen;
    CatalogoApp* app;

public:
    PortadaBox(int x, int y, std::shared_ptr<Video> v, CatalogoApp* aplicacion) 
        : Fl_Box(x, y, 120, 160), video(v), imagen(nullptr), app(aplicacion) {
        box(FL_BORDER_BOX);
        color(FL_BLACK);
        labelcolor(FL_WHITE);
        
        std::string rutaPortada = video->getRutaPortadaODefault();
        if (!rutaPortada.empty()) {
            imagen = Fl_Shared_Image::get(rutaPortada.c_str());
            if (imagen) {
                image(imagen);
            }
        }
        
        if (!imagen) {
            label("Sin\nImagen");
            align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
        }
    }
    
    ~PortadaBox() {
        if (imagen) imagen->release();
    }
    
    std::shared_ptr<Video> getVideo() const { return video; }
    
    int handle(int event) override {
        switch(event) {
            case FL_PUSH:
                if (Fl::event_button() == FL_LEFT_MOUSE) {
                    reproducirVideo();
                    return 1;
                }
                break;
            case FL_ENTER:
                box(FL_BORDER_FRAME);
                redraw();
                return 1;
            case FL_LEAVE:
                box(FL_BORDER_BOX);
                redraw();
                return 1;
        }
        return Fl_Box::handle(event);
    }
    
private:
    void reproducirVideo() {
        if (!video) return;
        
        if (video->getTipo() == "Pelicula") {
            std::string rutaVideo = video->getRutaVideo();
            std::string comando = "start \"\" \"" + rutaVideo + "\"";
            int resultado = system(comando.c_str());
            
            if (resultado == 0) {
                fl_message("Reproduciendo: %s", video->getTitulo().c_str());
            } else {
                fl_alert("No se pudo reproducir la película.\nVerifica que el archivo existe en: %s", rutaVideo.c_str());
            }
        } 
        else if (video->getTipo() == "Serie") {
            fl_message("Serie: %s\nUsa la opción 3 del menú para seleccionar episodios.", video->getTitulo().c_str());
        }
    }
};

// Ventana emergente para selecciones
class SelectorWindow : public Fl_Window {
private:
    Fl_Choice* choice;
    Fl_Button* okBtn;
    Fl_Button* cancelBtn;
    std::string seleccion;
    bool cancelado;

public:
    SelectorWindow(const std::string& titulo, const std::vector<std::string>& opciones)
        : Fl_Window(300, 150, titulo.c_str()), cancelado(true) {
        color(FL_BLACK);
        
        choice = new Fl_Choice(20, 30, 260, 30, "Seleccionar:");
        choice->color(FL_DARK3);
        choice->textcolor(FL_WHITE);
        for (const auto& op : opciones) {
            choice->add(op.c_str());
        }
        choice->value(0);
        
        okBtn = new Fl_Button(100, 80, 80, 30, "Aceptar");
        okBtn->color(FL_DARK2);
        okBtn->labelcolor(FL_WHITE);
        okBtn->callback(okCallback, this);
        
        cancelBtn = new Fl_Button(190, 80, 80, 30, "Cancelar");
        cancelBtn->color(FL_DARK2);
        cancelBtn->labelcolor(FL_WHITE);
        cancelBtn->callback(cancelCallback, this);
        
        end();
    }
    
    std::string getSeleccion() const { return seleccion; }
    bool fueCancelado() const { return cancelado; }
    
private:
    static void okCallback(Fl_Widget*, void* data) {
        SelectorWindow* win = static_cast<SelectorWindow*>(data);
        if (win->choice->value() >= 0) {
            win->seleccion = win->choice->text();
            win->cancelado = false;
        }
        win->hide();
    }
    
    static void cancelCallback(Fl_Widget*, void* data) {
        SelectorWindow* win = static_cast<SelectorWindow*>(data);
        win->cancelado = true;
        win->hide();
    }
};

// Clase principal de la aplicación
class CatalogoApp {
private:
    HistorialManager historial;
    std::vector<std::shared_ptr<Video>> catalogo;
    Fl_Window* window;
    Fl_Choice* menuChoice;
    Fl_Button* ejecutarBtn;
    Fl_Input* filtroInput;
    Fl_Spinner* calificacionSpinner;
    Fl_Text_Display* resultadosDisplay;
    Fl_Text_Buffer* textBuffer;
    Fl_Scroll* scrollPortadas;
    Fl_Pack* packPortadas;
    std::vector<PortadaBox*> portadas;

    void guardarHistorialAlCerrar() {
        historial.actualizarHistorialCompleto(catalogo);
    }

    void procesarArchivoDatos(const std::string& rutaArchivo) {
        std::ifstream archivo(rutaArchivo);
        if (!archivo.is_open()) {
            fl_alert("No se pudo abrir el archivo: %s", rutaArchivo.c_str());
            return;
        }
        
        std::string linea;
        int lineasProcesadas = 0;
        int calificacionesActualizadas = 0;
        int videosAgregados = 0;
        std::string errores;
        
        while (std::getline(archivo, linea)) {
            if (linea.empty() || linea[0] == '#') continue;
            
            lineasProcesadas++;
            std::vector<std::string> partes = dividirCadena(linea, '|');
            if (partes.empty()) continue;
            
            std::string tipo = partes[0];
            
            try {
                if (tipo == "CALIFICACION" && partes.size() >= 3) {
                    if (actualizarCalificacionExistente(partes[1], std::stod(partes[2]))) {
                        calificacionesActualizadas++;
                    }
                }
                else if (tipo == "PELICULA" && partes.size() >= 7) {
                    agregarNuevaPelicula(partes);
                    videosAgregados++;
                }
                else if (tipo == "SERIE" && partes.size() >= 8) {
                    agregarNuevaSerie(partes);
                    videosAgregados++;
                }
                else if (tipo == "USUARIO_CALIFICACION" && partes.size() >= 4) {
                    procesarCalificacionUsuario(partes[1], partes[2], std::stoi(partes[3]));
                }
                else if (tipo == "GENERO" && partes.size() >= 3) {
                    actualizarGeneroVideo(partes[1], partes[2]);
                }
            }
            catch (const std::exception& e) {
                errores += "Error en linea " + std::to_string(lineasProcesadas) + 
                          ": " + linea + " (" + e.what() + ")\n";
            }
        }
        
        archivo.close();
        actualizarPortadas();
        
        std::ostringstream resumen;
        resumen << "=== ARCHIVO PROCESADO EXITOSAMENTE ===\n\n";
        resumen << "Archivo: " << rutaArchivo << "\n";
        resumen << "Lineas procesadas: " << lineasProcesadas << "\n";
        resumen << "Calificaciones actualizadas: " << calificacionesActualizadas << "\n";
        resumen << "Videos agregados: " << videosAgregados << "\n";
        resumen << "Total videos en catalogo: " << catalogo.size() << "\n\n";
        
        if (!errores.empty()) {
            resumen << "=== ERRORES ENCONTRADOS ===\n" << errores;
        }
        
        resumen << "\n=== ESTADISTICAS DEL CATALOGO ===\n";
        resumen << generarEstadisticas();
        
        textBuffer->text(resumen.str().c_str());
    }
    
    std::vector<std::string> dividirCadena(const std::string& cadena, char separador) {
        std::vector<std::string> resultado;
        std::stringstream ss(cadena);
        std::string item;
        
        while (std::getline(ss, item, separador)) {
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);
            resultado.push_back(item);
        }
        
        return resultado;
    }
    
    bool actualizarCalificacionExistente(const std::string& titulo, double nuevaCalificacion) {
        for (auto& video : catalogo) {
            if (video->getTitulo() == titulo) {
                video->setCalificacion(nuevaCalificacion);
                return true;
            }
        }
        return false;
    }
    
    void agregarNuevaPelicula(const std::vector<std::string>& partes) {
        std::string titulo = partes[1];
        double calificacion = std::stod(partes[2]);
        int duracion = std::stoi(partes[3]);
        std::string genero = partes[4];
        std::string director = partes[5];                
        int anio = std::stoi(partes[6]);
        
        for (auto& video : catalogo) {
            if (video->getTitulo() == titulo) return;
        }
        
        catalogo.push_back(std::make_shared<Pelicula>(
            titulo, calificacion, duracion, genero, director, anio));
    }
    
    void agregarNuevaSerie(const std::vector<std::string>& partes) {
        std::string titulo = partes[1];
        double calificacion = std::stod(partes[2]);
        int episodiosPorTemp = std::stoi(partes[3]);
        std::string genero = partes[4];
        int numTemporadas = std::stoi(partes[5]);
        int totalEpisodios = std::stoi(partes[6]);
        std::string director = partes[7];
        
        for (auto& video : catalogo) {
            if (video->getTitulo() == titulo) return;
        }
        
        catalogo.push_back(std::make_shared<Serie>(
            titulo, calificacion, episodiosPorTemp, genero,
            numTemporadas, totalEpisodios, director));
    }
    
    void procesarCalificacionUsuario(const std::string& usuario, 
                                    const std::string& titulo, 
                                    int calificacion) {
        for (auto& video : catalogo) {
            if (video->getTitulo() == titulo) {
                video->actualizarCalificacion(calificacion);
                break;
            }
        }
    }
    
    void actualizarGeneroVideo(const std::string& titulo, const std::string& nuevoGenero) {
        for (auto& video : catalogo) {
            if (video->getTitulo() == titulo) {
                video->setGenero(nuevoGenero);
            }
        }
    }
    
    std::string generarEstadisticas() {
        int totalPeliculas = 0;
        int totalSeries = 0;
        double sumaCalificaciones = 0;
        std::map<std::string, int> generos;
        std::map<std::string, int> directores;
        
        for (auto& video : catalogo) {
            if (video->getTipo() == "Pelicula") totalPeliculas++;
            else totalSeries++;
            
            sumaCalificaciones += video->getCalificacion();
            generos[video->getGenero()]++;
            directores[video->getDirector()]++;
        }
        
        double promedioCalificacion = catalogo.empty() ? 0 : sumaCalificaciones / catalogo.size();
        
        std::string generoMasPopular = "N/A";
        int maxGenero = 0;
        for (const auto& par : generos) {
            if (par.second > maxGenero) {
                maxGenero = par.second;
                generoMasPopular = par.first;
            }
        }
        
        std::string directorMasRepresentado = "N/A";
        int maxDirector = 0;
        for (const auto& par : directores) {
            if (par.second > maxDirector) {
                maxDirector = par.second;
                directorMasRepresentado = par.first;
            }
        }
        
        std::ostringstream stats;
        stats << "Películas: " << totalPeliculas << "\n";
        stats << "Series: " << totalSeries << "\n";
        stats << "Total videos: " << catalogo.size() << "\n";
        stats << "Calificación promedio: " << std::fixed << std::setprecision(1) << promedioCalificacion << "\n";
        stats << "Género más popular: " << generoMasPopular << " (" << maxGenero << " videos)\n";
        stats << "Director más representado: " << directorMasRepresentado << " (" << maxDirector << " videos)\n";
        
        return stats.str();
    }

    void mostrarPeliculasSoloCalificacion() {
        try {
            std::vector<std::string> rangos = {"1-2", "3-4", "5-6", "7-8", "9-10"};
            SelectorWindow rangoWin("Seleccionar Rango de Calificación", rangos);
            rangoWin.show();
            while (rangoWin.shown()) Fl::wait();
            if (rangoWin.fueCancelado()) {
                textBuffer->text("Operación cancelada.");
                return;
            }
            std::string rangoSeleccionado = rangoWin.getSeleccion();

            int rangoMin = 0, rangoMax = 10;
            if (rangoSeleccionado == "1-2") { rangoMin = 1; rangoMax = 2; }
            else if (rangoSeleccionado == "3-4") { rangoMin = 3; rangoMax = 4; }
            else if (rangoSeleccionado == "5-6") { rangoMin = 5; rangoMax = 6; }
            else if (rangoSeleccionado == "7-8") { rangoMin = 7; rangoMax = 8; }
            else if (rangoSeleccionado == "9-10") { rangoMin = 9; rangoMax = 10; }

            std::vector<std::shared_ptr<Video>> videosFiltrados;
            std::ostringstream oss;
            oss << "Películas y Series en el rango de calificación " << rangoSeleccionado << ":\n\n";

            for (const auto& video : catalogo) {
                if (video) {
                    int primeraCifraCalificacion = static_cast<int>(video->getCalificacion());
                    if (primeraCifraCalificacion >= rangoMin && primeraCifraCalificacion <= rangoMax) {
                        videosFiltrados.push_back(video);
                        oss << video->getInfo() << "\n\n";
                    }
                }
            }

            if (videosFiltrados.empty()) {
                oss << "No se encontraron videos en el rango " << rangoSeleccionado << ".";
            }

            actualizarPortadas(videosFiltrados);
            textBuffer->text(oss.str().c_str());
        } catch (const std::exception& e) {
            fl_alert("Error al mostrar películas: %s", e.what());
            textBuffer->text("Error al mostrar películas.");
        }
    }

public:
    CatalogoApp() {
        setupUI();
        cargarDatosPorDefecto();
        historial.cargarHistorial(catalogo);
        actualizarPortadas();
    }
    
    ~CatalogoApp() {
        delete textBuffer;
        for (auto* portada : portadas) {
            delete portada;
        }
    }

    void run() {
        window->show();
        Fl::run();
    }
    
    void reproducirSerie(const std::string& titulo) {
        mostrarEpisodiosSerie();
    }   

    void ordenarPorCalificacion() {
        std::sort(catalogo.begin(), catalogo.end(), 
            [](const auto& a, const auto& b) { return *a > *b; });
    }

    void ajustarCalificaciones() {
        try {
            std::vector<std::string> titulos;
            for (const auto& video : catalogo) {
                if (video) titulos.push_back(video->getTitulo());
            }
        
            SelectorWindow tituloWin("Seleccionar Video", titulos);
            tituloWin.show();
            while (tituloWin.shown()) Fl::wait();
            if (tituloWin.fueCancelado()) return;
        
            std::string tituloSeleccionado = tituloWin.getSeleccion();
        
            std::vector<std::string> operaciones = {"Aumentar +0.5", "Disminuir -0.5", "Aumentar +1.0", "Disminuir -1.0"};
            SelectorWindow opWin("Seleccionar Operación", operaciones);
            opWin.show();
            while (opWin.shown()) Fl::wait();
            if (opWin.fueCancelado()) return;
        
            std::string operacion = opWin.getSeleccion();
        
            for (auto& video : catalogo) {
                if (video->getTitulo() == tituloSeleccionado) {
                    double calAnterior = video->getCalificacion();
                
                    if (operacion == "Aumentar +0.5") {
                        *video += 0.5;
                    } else if (operacion == "Disminuir -0.5") {
                        *video -= 0.5;
                    } else if (operacion == "Aumentar +1.0") {
                        *video += 1.0;
                    } else if (operacion == "Disminuir -1.0") {
                        *video -= 1.0;
                    }
                
                    std::ostringstream oss;
                    oss << "Calificación ajustada:\n";
                    oss << "Video: " << video->getTitulo() << "\n";
                    oss << "Calificación anterior: " << std::fixed << std::setprecision(1) << calAnterior << "\n";
                    oss << "Nueva calificación: " << std::fixed << std::setprecision(1) << video->getCalificacion() << "\n";
                
                    textBuffer->text(oss.str().c_str());
                    actualizarPortadas();
                    break;
                }
            }
        
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void mostrarMejorCalificado() {
        if (catalogo.empty()) {
            textBuffer->text("No hay videos en el catálogo.");
            return;
        }
    
        auto mejor = *std::max_element(catalogo.begin(), catalogo.end(),
            [](const auto& a, const auto& b) { return *a < *b; });
    
        std::ostringstream oss;
        oss << "Video con mejor calificación:\n\n";
        oss << *mejor;
        textBuffer->text(oss.str().c_str());
    }

    void mostrarVideosSimilares() {
        try {
            std::vector<std::string> titulos;
            for (const auto& video : catalogo) {
                if (video) titulos.push_back(video->getTitulo());
            }
        
            SelectorWindow tituloWin("Seleccionar Video Base", titulos);
            tituloWin.show();
            while (tituloWin.shown()) Fl::wait();
            if (tituloWin.fueCancelado()) return;
        
            std::string tituloSeleccionado = tituloWin.getSeleccion();
            std::shared_ptr<Video> videoBase = nullptr;
        
            for (const auto& video : catalogo) {
                if (video->getTitulo() == tituloSeleccionado) {
                    videoBase = video;
                    break;
                }
            }
        
            if (!videoBase) return;
        
            std::ostringstream oss;
            oss << "Videos similares a: " << videoBase->getTitulo() << "\n";
            oss << "Calificación base: " << videoBase->getCalificacion() << "\n\n";
        
            for (const auto& video : catalogo) {
                if (video != videoBase) {
                    if (*video > *videoBase) {
                        oss << "MEJOR: " << *video << "\n\n";
                    } else if (*video < *videoBase) {
                        oss << "MENOR: " << *video << "\n\n";
                    } else {
                        oss << "IGUAL: " << *video << "\n\n";
                    }
                }
            }
        
            textBuffer->text(oss.str().c_str());
        
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }  

private:
    void setupUI() {
        window = new Fl_Window(1000, 700, "Catalogo de Películas y Series");
        window->color(FL_BLACK);
            
        menuChoice = new Fl_Choice(20, 20, 300, 30, "Menú:");
        menuChoice->add("1. Cargar archivo de datos");
        menuChoice->add("2. Mostrar videos por género o calificación");
        menuChoice->add("3. Mostrar episodios de serie");
        menuChoice->add("4. Mostrar películas por calificación");
        menuChoice->add("5. Calificar video");
        menuChoice->add("6. Ordenar por calificación");
        menuChoice->add("7. Mostrar mejor calificado");
        menuChoice->add("8. Comparar videos");
        menuChoice->add("0. Salir");
        menuChoice->value(0);
        menuChoice->color(FL_DARK3);
        menuChoice->textcolor(FL_WHITE);
        
        ejecutarBtn = new Fl_Button(330, 20, 80, 30, "Ejecutar");
        ejecutarBtn->color(FL_DARK2);
        ejecutarBtn->labelcolor(FL_WHITE);
        ejecutarBtn->callback(ejecutarCallback, this);
        
        filtroInput = new Fl_Input(480, 20, 150, 30, "Filtro:");
        filtroInput->color(FL_DARK3);
        filtroInput->textcolor(FL_WHITE);
        
        calificacionSpinner = new Fl_Spinner(700, 20, 80, 30, "Cal. mín:");
        calificacionSpinner->minimum(0);
        calificacionSpinner->maximum(10);
        calificacionSpinner->value(0);
        calificacionSpinner->color(FL_DARK3);
        calificacionSpinner->textcolor(FL_WHITE);
        
        scrollPortadas = new Fl_Scroll(20, 70, 960, 300);
        scrollPortadas->color(FL_BLACK);
        
        packPortadas = new Fl_Pack(25, 75, 950, 290);
        packPortadas->type(Fl_Pack::HORIZONTAL);
        packPortadas->spacing(10);
        
        scrollPortadas->end();
        
        Fl_Box* labelResultados = new Fl_Box(20, 380, 100, 20, "Resultados:");
        labelResultados->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        labelResultados->labelcolor(FL_WHITE);
        labelResultados->color(FL_BLACK);
        
        resultadosDisplay = new Fl_Text_Display(20, 400, 960, 280);
        textBuffer = new Fl_Text_Buffer();
        resultadosDisplay->buffer(textBuffer);
        resultadosDisplay->color(FL_DARK3);
        resultadosDisplay->textcolor(FL_WHITE);
        
        window->end();
    }
    
    static void ejecutarCallback(Fl_Widget* w, void* data) {
        CatalogoApp* app = static_cast<CatalogoApp*>(data);
        app->ejecutarOpcion();
    }
    
    void ejecutarOpcion() {
        int opcion = menuChoice->value();
    
        try {
            switch(opcion) {
                case 0: cargarArchivoDatos(); break;
                case 1: mostrarVideosPorCalificacionOGenero(); break;
                case 2: mostrarEpisodiosSerie(); break;
                case 3: mostrarPeliculasSoloCalificacion(); break;
                case 4: calificarVideo(); break;
                case 5:
                    ordenarPorCalificacion();
                    actualizarPortadas();
                    textBuffer->text("Catálogo ordenado por calificación (mayor a menor)");
                    break;
                case 6:
                    mostrarMejorCalificado();
                    break;
                case 7:
                    mostrarVideosSimilares();
                    break;
                case 8:
                    guardarHistorialAlCerrar();
                    window->hide();
                    Fl::delete_widget(window);
                    Fl::check();
                    exit(0);
                    break;
                default:
                    fl_alert("Opción no válida seleccionada.");
                    break;
            }
        } catch (const std::exception& e) {
            fl_alert("Error al ejecutar opción: %s", e.what());
        }
    }
    
    void cargarDatosPorDefecto() {
        catalogo.push_back(std::make_shared<Pelicula>(
            "La princesa Mononoke", 8.4, 134, "Fantasia",
            "Hayao Miyazaki", 1997));
        catalogo.push_back(std::make_shared<Pelicula>(
            "El viaje de Chihiro", 8.6, 125, "Fantasia",
            "Hayao Miyazaki", 2001));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Look Back", 8.1, 90, "Drama",
            "Kiyotaka Oshiyama", 2021));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio I La amenaza fantasma", 6.5, 136, "Ciencia Ficcion",
            "George Lucas", 1999));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio II El ataque de los clones", 6.5, 142, "Ciencia Ficcion",
            "George Lucas", 2002));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio III La venganza de los Sith", 7.5, 140, "Ciencia Ficcion",
            "George Lucas", 2005));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio IV Una nueva esperanza", 8.6, 121, "Ciencia Ficcion",
            "George Lucas", 1977));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio V El imperio contraataca", 8.7, 124, "Ciencia Ficcion",
            "Irvin Kershner", 1980));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio VI El retorno del Jedi", 8.3, 131, "Ciencia Ficcion",
            "Richard Marquand", 1983));
        
        catalogo.push_back(std::make_shared<Serie>(
            "Jujutsu Kaisen", 8.7, 24, "Accion",
            2, 47, "Sunghoo Park"));
        catalogo.push_back(std::make_shared<Serie>(
            "Pokemon", 7.5, 22, "Aventura",
            25, 1200, "Kunihiko Yuyama"));
        catalogo.push_back(std::make_shared<Serie>(
            "Violet Evergarden", 8.8, 24, "Drama",
            1, 13, "Taichi Ishidate"));
        catalogo.push_back(std::make_shared<Serie>(
            "Kimetsu no Yaiba", 8.7, 24, "Accion",
            3, 55, "Haruo Sotozaki"));
        catalogo.push_back(std::make_shared<Serie>(
            "Attack on Titan", 9.0, 24, "Accion",
            4, 87, "Tetsuro Araki"));
        catalogo.push_back(std::make_shared<Serie>(
            "Blue Lock", 8.3, 24, "Deporte",
            1, 24, "Tetsuaki Watanabe"));
        catalogo.push_back(std::make_shared<Serie>(
            "Star Wars The Clone Wars", 8.4, 22, "Ciencia Ficcion",
            7, 133, "Dave Filoni"));
        catalogo.push_back(std::make_shared<Serie>(
            "Ann", 7.9, 45, "Drama",
            1, 10, "Unknown"));
        catalogo.push_back(std::make_shared<Serie>(
            "Nadie nos va a extrañar", 8.1, 45, "Crimen",
            1, 10, "Unknown"));
        catalogo.push_back(std::make_shared<Serie>(
            "Si la vida te da mandarinas", 7.8, 45, "Comedia",
            1, 10, "Unknown"));
        catalogo.push_back(std::make_shared<Serie>(
            "Goblin", 8.9, 70, "Romance",
            1, 16, "Lee Eung-bok"));
        catalogo.push_back(std::make_shared<Serie>(
            "Alien Stage", 8.5, 15, "Musical",
            1, 6, "Unknown"));
    }
    
    void actualizarPortadas(const std::vector<std::shared_ptr<Video>>& videosFiltrados = {}) {
        try {
            for (auto* portada : portadas) {
                packPortadas->remove(portada);
                delete portada;
            }
            portadas.clear();
            
            const auto& videos = videosFiltrados.empty() ? catalogo : videosFiltrados;
            int x = 25;
            for (const auto& video : videos) {
                if (video) {
                    PortadaBox* portada = new PortadaBox(x, 75, video, this);
                    portadas.push_back(portada);
                    packPortadas->add(portada);
                    x += 130;
                }
            }
            
            packPortadas->redraw();
            scrollPortadas->redraw();
        } catch (const std::exception& e) {
            fl_alert("Error al actualizar portadas: %s", e.what());
        }
    }
    
    void cargarArchivoDatos() {
        const char* filename = fl_file_chooser("Seleccionar archivo de datos", "*.txt", "");
        if (filename) {
            procesarArchivoDatos(filename);
        }
    }
    
    void mostrarVideosPorCalificacionOGenero() {
        try {
            std::vector<std::string> tiposFiltro = {"Por Genero", "Por Calificacion"};
            SelectorWindow tipoWin("Seleccionar Tipo de Filtro", tiposFiltro);
            tipoWin.show();
            while (tipoWin.shown()) Fl::wait();
            if (tipoWin.fueCancelado()) {
                textBuffer->text("Operación cancelada.");
                return;
            }
            std::string tipoSeleccionado = tipoWin.getSeleccion();
            
            std::vector<std::shared_ptr<Video>> videosFiltrados;
            std::ostringstream resultado;
            
            if (tipoSeleccionado == "Por Genero") {
                std::set<std::string> generos;
                for (const auto& video : catalogo) {
                    if (video) generos.insert(video->getGenero());
                }
                std::vector<std::string> listaGeneros(generos.begin(), generos.end());
                
                SelectorWindow genWin("Seleccionar Genero", listaGeneros);
                genWin.show();
                while (genWin.shown()) Fl::wait();
                if (genWin.fueCancelado()) {
                    textBuffer->text("Operación cancelada.");
                    return;
                }
                std::string generoSeleccionado = genWin.getSeleccion();
                
                resultado << "Videos del género \"" << generoSeleccionado << "\":\n\n";
                
                for (const auto& video : catalogo) {
                    if (video && video->getGenero() == generoSeleccionado) {
                        videosFiltrados.push_back(video);
                        resultado << video->getInfo() << "\n\n";
                    }
                }
                
            } else if (tipoSeleccionado == "Por Calificacion") {
                std::vector<std::string> rangos = {"1-2", "3-4", "5-6", "7-8", "9-10"};
                SelectorWindow rangoWin("Seleccionar Rango de Calificacion", rangos);
                rangoWin.show();
                while (rangoWin.shown()) Fl::wait();
                if (rangoWin.fueCancelado()) {
                    textBuffer->text("Operación cancelada.");
                    return;
                }
                std::string rangoSeleccionado = rangoWin.getSeleccion();
                
                double calMin = 0, calMax = 10;
                if (rangoSeleccionado == "1-2") { calMin = 1.0; calMax = 2.0; }
                else if (rangoSeleccionado == "3-4") { calMin = 3.0; calMax = 4.0; }
                else if (rangoSeleccionado == "5-6") { calMin = 5.0; calMax = 6.0; }
                else if (rangoSeleccionado == "7-8") { calMin = 7.0; calMax = 8.0; }
                else if (rangoSeleccionado == "9-10") { calMin = 9.0; calMax = 10.0; }
                
                resultado << "Videos con calificación en el rango " << rangoSeleccionado << ":\n\n";
                
                for (const auto& video : catalogo) {
                    if (video && video->getCalificacion() >= calMin && video->getCalificacion() <= calMax) {
                        videosFiltrados.push_back(video);
                        resultado << video->getInfo() << "\n\n";
                    }
                }
            }
            
            if (videosFiltrados.empty()) {
                resultado << "No se encontraron videos que cumplan con el criterio seleccionado.";
            }
            
            textBuffer->text(resultado.str().c_str());
            
        } catch (const std::exception& e) {
            fl_alert("Error al mostrar videos: %s", e.what());
            textBuffer->text("Error al mostrar videos.");
        }
    }
    
    void mostrarEpisodiosSerie() {
        try {
            std::vector<std::string> series;
            for (const auto& video : catalogo) {
                if (video && video->getTipo() == "Serie") {
                    series.push_back(video->getTitulo());
                }
            }
            
            if (series.empty()) {
                textBuffer->text("No hay series disponibles en el catálogo.");
                return;
            }
            
            SelectorWindow serieWin("Seleccionar Serie", series);
            serieWin.show();
            while (serieWin.shown()) Fl::wait();
            if (serieWin.fueCancelado()) {
                textBuffer->text("Operación cancelada.");
                return;
            }
            std::string serieSeleccionada = serieWin.getSeleccion();
            
            std::shared_ptr<Serie> serieEncontrada = nullptr;
            for (const auto& video : catalogo) {
                if (video && video->getTipo() == "Serie" && video->getTitulo() == serieSeleccionada) {
                    serieEncontrada = std::dynamic_pointer_cast<Serie>(video);
                    break;
                }
            }
            
            if (!serieEncontrada) {
                textBuffer->text("Error: No se pudo encontrar la serie seleccionada.");
                return;
            }
            
            std::vector<std::string> episodios;
            int epNum = 1;
            for (int temp = 1; temp <= serieEncontrada->getNumTemporadas(); temp++) {
                int epsEstaTemporada = std::min(serieEncontrada->getEpisodiosPorTemporada(), 
                                              serieEncontrada->getTotalEpisodios() - (epNum - 1));
                for (int ep = 1; ep <= epsEstaTemporada; ep++, epNum++) {
                    std::ostringstream episodioStr;
                    episodioStr << "T" << temp << "E" << ep << " - Episodio " << epNum;
                    episodios.push_back(episodioStr.str());
                }
            }
            
            SelectorWindow episodioWin("Seleccionar Episodio para Reproducir", episodios);
            episodioWin.show();
            while (episodioWin.shown()) Fl::wait();
            if (episodioWin.fueCancelado()) {
                textBuffer->text("Operación cancelada.");
                return;
            }
            std::string episodioSeleccionado = episodioWin.getSeleccion();
            
            std::string nombreArchivo = tituloANombreArchivo(serieSeleccionada);
            
            std::string temp_ep = episodioSeleccionado.substr(0, episodioSeleccionado.find(" - "));
            std::string episodio_formato = temp_ep;
            std::transform(episodio_formato.begin(), episodio_formato.end(), episodio_formato.begin(), ::tolower);
            episodio_formato[0] = 's';
            
            std::string rutaEpisodio = ".\\videos\\series\\" + nombreArchivo + "_" + episodio_formato + ".mp4";
            
            std::string comando = "start \"\" \"" + rutaEpisodio + "\"";
            int resultado = system(comando.c_str());
            
            std::ostringstream info;
            info << "=== INFORMACIÓN DEL EPISODIO ===\n\n";
            info << "Serie: " << serieSeleccionada << "\n";
            info << "Episodio: " << episodioSeleccionado << "\n";
            info << "Ruta: " << rutaEpisodio << "\n\n";
            info << "=== INFORMACIÓN DE LA SERIE ===\n";
            info << serieEncontrada->getInfo() << "\n\n";
            
            if (resultado == 0) {
                info << "Estado: Intentando reproducir episodio...\n";
                info << "Nota: Si el archivo no se encuentra, verifica que exista en la ruta especificada.";
            } else {
                info << "Estado: Error al intentar abrir el archivo.\n";
                info << "Verifica que el archivo existe y que tienes un reproductor de video configurado.";
            }
            
            textBuffer->text(info.str().c_str());
            
        } catch (const std::exception& e) {
            fl_alert("Error al mostrar episodios: %s", e.what());
            textBuffer->text("Error al mostrar episodios.");
        }
    }
    
    void calificarVideo() {
        try {
            std::vector<std::string> titulos;
            for (const auto& video : catalogo) {
                if (video) titulos.push_back(video->getTitulo());
            }
        
            SelectorWindow tituloWin("Seleccionar Video para Calificar", titulos);
            tituloWin.show();
            while (tituloWin.shown()) Fl::wait();
            if (tituloWin.fueCancelado()) {
                textBuffer->text("Operación cancelada.");
                return;
            }
            std::string tituloSeleccionado = tituloWin.getSeleccion();
        
            const char* input = fl_input("Ingresa la calificación (1-10):", "5");
            if (input) {
                int calificacion = std::stoi(input);
                if (calificacion >= 1 && calificacion <= 10) {
                    for (auto& video : catalogo) {
                        if (video && video->getTitulo() == tituloSeleccionado) {
                            double calificacionAnterior = video->getCalificacion();
                            video->actualizarCalificacion(calificacion);
                        
                            // Guardar el historial completo actualizado
                            historial.actualizarHistorialCompleto(catalogo);
                        
                            std::ostringstream oss;
                            oss << "Calificación actualizada para: " << video->getTitulo() 
                                << "\nCalificación anterior: " << std::fixed << std::setprecision(1) << calificacionAnterior
                                << "\nNueva calificación: " << std::fixed << std::setprecision(1) << video->getCalificacion()
                                << "\n\nHistorial guardado en: historialDatos.txt";
                        
                            textBuffer->text(oss.str().c_str());
                            fl_message("Calificación guardada exitosamente");
                            actualizarPortadas();
                            return;
                        }
                    }
                    fl_alert("No se encontró el video");
                } else {
                    fl_alert("La calificación debe estar entre 1 y 10");
                }
            }
        } catch (const std::exception& e) {
            fl_alert("Error al calificar video: %s", e.what());
            textBuffer->text("Error al calificar video.");
        }
    }
};

int main() {
    try {
        fl_register_images();
        CatalogoApp app;
        app.run();
        return 0;
    } catch (const std::exception& e) {
        fl_alert("Error en la aplicación: %s", e.what());
        return 1;
    }
}