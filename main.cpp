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
    Video(const std::string& t, double cal, const std::string& g, 
          const std::string& dir, int a)
        : titulo(t), calificacion(cal), genero(g), director(dir), 
          anio(a), basePath(".\\") {
        std::string nombreArchivo = tituloANombreArchivo(titulo);
        rutaPortada = basePath + "portadas\\" + nombreArchivo + ".jpg";
    }
    
    virtual ~Video() = default;
    
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

public:
    PortadaBox(int x, int y, std::shared_ptr<Video> v) 
        : Fl_Box(x, y, 120, 160), video(v), imagen(nullptr) {
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
    
    int handle(int event) override {
        if (event == FL_PUSH) {
            reproducirVideo();
            return 1;
        }
        return Fl_Box::handle(event);
    }
    
    void reproducirVideo() {
        std::string rutaVideo = video->getRutaVideo();
        std::string comando;
        
        std::ifstream file(rutaVideo);
        if (!file.good()) {
            fl_alert("No se encontro el archivo de video:\n%s", rutaVideo.c_str());
            return;
        }
        
        #ifdef _WIN32
            comando = "start \"\" \"" + rutaVideo + "\"";
        #elif __APPLE__
            comando = "open \"" + rutaVideo + "\"";
        #else
            comando = "xdg-open \"" + rutaVideo + "\"";
        #endif
        
        system(comando.c_str());
        fl_message("Reproduciendo:\n%s\n\nRuta: %s", video->getInfo().c_str(), rutaVideo.c_str());
    }
    
    std::shared_ptr<Video> getVideo() const { return video; }
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

public:
    CatalogoApp() {
        setupUI();
        cargarDatosPorDefecto();
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

private:
    void setupUI() {
        window = new Fl_Window(1000, 700, "Catalogo de Películas y Series");
        window->color(FL_BLACK);
        
        menuChoice = new Fl_Choice(20, 20, 300, 30, "Menú:");
        menuChoice->add("1. Cargar archivo de datos");
        menuChoice->add("2. Mostrar videos por calificación/género");
        menuChoice->add("3. Mostrar episodios de serie");
        menuChoice->add("4. Mostrar películas por calificación");
        menuChoice->add("5. Calificar video");
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
        
        switch(opcion) {
            case 0: cargarArchivoDatos(); break;
            case 1: mostrarVideosPorCalificacionOGenero(); break;
            case 2: mostrarEpisodiosSerie(); break;
            case 3: mostrarPeliculasPorCalificacion(); break;
            case 4: calificarVideo(); break;
            case 5: 
                window->hide();
                Fl::delete_widget(window);
                Fl::check();
                exit(0);
                break;
        }
    }
    
    void cargarDatosPorDefecto() {
        catalogo.push_back(std::make_shared<Pelicula>(
            "La princesa Mononoke", 8.4, 134, "Fantasia/Aventura", 
            "Hayao Miyazaki", 1997));
        catalogo.push_back(std::make_shared<Pelicula>(
            "El viaje de Chihiro", 8.6, 125, "Fantasia/Aventura", 
            "Hayao Miyazaki", 2001));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Look Back", 8.1, 90, "Drama/Slice of Life", 
            "Kiyotaka Oshiyama", 2021));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio I La amenaza fantasma", 6.5, 136, "Ciencia ficcion", 
            "George Lucas", 1999));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio II El ataque de los clones", 6.5, 142, "Ciencia ficcion", 
            "George Lucas", 2002));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio III La venganza de los Sith", 7.5, 140, "Ciencia ficcion", 
            "George Lucas", 2005));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio IV Una nueva esperanza", 8.6, 121, "Ciencia ficcion", 
            "George Lucas", 1977));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio V El imperio contraataca", 8.7, 124, "Ciencia ficcion", 
            "Irvin Kershner", 1980));
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars Episodio VI El retorno del Jedi", 8.3, 131, "Ciencia ficcion", 
            "Richard Marquand", 1983));
        catalogo.push_back(std::make_shared<Serie>(
            "Jujutsu Kaisen", 8.7, 24, "Accion/Fantasia oscura", 
            2, 47, "Sunghoo Park"));
        catalogo.push_back(std::make_shared<Serie>(
            "Pokemon", 7.5, 22, "Aventura/Fantasia", 
            25, 1200, "Kunihiko Yuyama"));
        catalogo.push_back(std::make_shared<Serie>(
            "Violet Evergarden", 8.8, 24, "Drama/Fantasia", 
            1, 13, "Taichi Ishidate"));
        catalogo.push_back(std::make_shared<Serie>(
            "Kimetsu no Yaiba", 8.7, 24, "Accion/Fantasia oscura", 
            3, 55, "Haruo Sotozaki"));
        catalogo.push_back(std::make_shared<Serie>(
            "Attack on Titan", 9.0, 24, "Accion/Fantasia oscura", 
            4, 87, "Tetsuro Araki"));
        catalogo.push_back(std::make_shared<Serie>(
            "Blue Lock", 8.3, 24, "Deporte/Drama", 
            1, 24, "Tetsuaki Watanabe"));
        catalogo.push_back(std::make_shared<Serie>(
            "Star Wars The Clone Wars", 8.4, 22, "Ciencia ficcion", 
            7, 133, "Dave Filoni"));
        catalogo.push_back(std::make_shared<Serie>(
            "Ann", 7.9, 45, "Drama/Thriller", 
            1, 10, "Unknown"));
        catalogo.push_back(std::make_shared<Serie>(
            "Nadie nos va a extrañar", 8.1, 45, "Drama/Crimen", 
            1, 10, "Unknown"));
        catalogo.push_back(std::make_shared<Serie>(
            "Si la vida te da mandarinas", 7.8, 45, "Drama/Comedia", 
            1, 10, "Unknown"));
        catalogo.push_back(std::make_shared<Serie>(
            "Goblin", 8.9, 70, "Fantasia/Romance", 
            1, 16, "Lee Eung-bok"));
        catalogo.push_back(std::make_shared<Serie>(
            "Alien Stage", 8.5, 15, "Ciencia ficcion/Musical", 
            1, 6, "Unknown"));
    }
    
    void actualizarPortadas(const std::vector<std::shared_ptr<Video>>& videosFiltrados = {}) {
        for (auto* portada : portadas) {
            packPortadas->remove(portada);
            delete portada;
        }
        portadas.clear();
        
        const auto& videos = videosFiltrados.empty() ? catalogo : videosFiltrados;
        int x = 25;
        for (const auto& video : videos) {
            PortadaBox* portada = new PortadaBox(x, 75, video);
            portadas.push_back(portada);
            packPortadas->add(portada);
            x += 130;
        }
        
        packPortadas->redraw();
        scrollPortadas->redraw();
    }
    
    void cargarArchivoDatos() {
        const char* filename = fl_file_chooser("Seleccionar archivo de datos", "*.txt", "");
        if (filename) {
            procesarArchivoDatos(filename);
        }
    }
    
    void mostrarVideosPorCalificacionOGenero() {
        // Obtener géneros únicos
        std::set<std::string> generos;
        for (const auto& video : catalogo) {
            generos.insert(video->getGenero());
        }
        std::vector<std::string> listaGeneros(generos.begin(), generos.end());
        
        // Ventana para seleccionar género
        SelectorWindow genWin("Seleccionar Genero", listaGeneros);
        genWin.show();
        while (genWin.shown()) Fl::wait();
        if (genWin.fueCancelado()) return;
        std::string generoSeleccionado = genWin.getSeleccion();
        
        // Ventana para seleccionar rango de calificación
        std::vector<std::string> rangos = {"1-2", "3-4", "5-6", "7-8", "9-10"};
        SelectorWindow rangoWin("Seleccionar Rango de Calificacion", rangos);
        rangoWin.show();
        while (rangoWin.shown()) Fl::wait();
        if (rangoWin.fueCancelado()) return;
        std::string rangoSeleccionado = rangoWin.getSeleccion();
        
        // Parsear rango
        double calMin = 0, calMax = 10;
        if (rangoSeleccionado == "1-2") { calMin = 1; calMax = 2; }
        else if (rangoSeleccionado == "3-4") { calMin = 3; calMax = 4; }
        else if (rangoSeleccionado == "5-6") { calMin = 5; calMax = 6; }
        else if (rangoSeleccionado == "7-8") { calMin = 7; calMax = 8; }
        else if (rangoSeleccionado == "9-10") { calMin = 9; calMax = 10; }
        
        // Filtrar videos
        std::vector<std::shared_ptr<Video>> videosFiltrados;
        std::string resultado = "Videos filtrados:\n\n";
        for (const auto& video : catalogo) {
            bool coincideGenero = video->getGenero() == generoSeleccionado;
            bool coincideCalif = video->getCalificacion() >= calMin && video->getCalificacion() <= calMax;
            if (coincideGenero && coincideCalif) {
                videosFiltrados.push_back(video);
                resultado += video->getInfo() + "\n\n";
            }
        }
        
        actualizarPortadas(videosFiltrados);
        textBuffer->text(resultado.c_str());
    }
    
    void mostrarEpisodiosSerie() {
        // Obtener series
        std::vector<std::string> series;
        for (const auto& video : catalogo) {
            if (video->getTipo() == "Serie") {
                series.push_back(video->getTitulo());
            }
        }
        
        // Ventana para seleccionar serie
        SelectorWindow serieWin("Seleccionar Serie", series);
        serieWin.show();
        while (serieWin.shown()) Fl::wait();
        if (serieWin.fueCancelado()) return;
        std::string serieSeleccionada = serieWin.getSeleccion();
        
        // Ventana para seleccionar rango de calificación
        std::vector<std::string> rangos = {"1-2", "3-4", "5-6", "7-8", "9-10"};
        SelectorWindow rangoWin("Seleccionar Rango de Calificacion", rangos);
        rangoWin.show();
        while (rangoWin.shown()) Fl::wait();
        if (rangoWin.fueCancelado()) return;
        std::string rangoSeleccionado = rangoWin.getSeleccion();
        
        // Parsear rango
        double calMin = 0, calMax = 10;
        if (rangoSeleccionado == "1-2") { calMin = 1; calMax = 2; }
        else if (rangoSeleccionado == "3-4") { calMin = 3; calMax = 4; }
        else if (rangoSeleccionado == "5-6") { calMin = 5; calMax = 6; }
        else if (rangoSeleccionado == "7-8") { calMin = 7; calMax = 8; }
        else if (rangoSeleccionado == "9-10") { calMin = 9; calMax = 10; }
        
        // Mostrar episodios
        std::string resultado = "Episodios de la serie seleccionada:\n\n";
        for (const auto& video : catalogo) {
            if (video->getTipo() == "Serie" && video->getTitulo() == serieSeleccionada) {
                auto serie = std::dynamic_pointer_cast<Serie>(video);
                if (serie->getCalificacion() >= calMin && serie->getCalificacion() <= calMax) {
                    resultado += serie->getInfo() + "\n\n";
                    resultado += serie->getEpisodiosInfo();
                    break;
                }
            }
        }
        textBuffer->text(resultado.c_str());
    }
    
    void mostrarPeliculasPorCalificacion() {
        // Ventana para seleccionar rango de calificación
        std::vector<std::string> rangos = {"1-2", "3-4", "5-6", "7-8", "9-10"};
        SelectorWindow rangoWin("Seleccionar Rango de Calificacion", rangos);
        rangoWin.show();
        while (rangoWin.shown()) Fl::wait();
        if (rangoWin.fueCancelado()) return;
        std::string rangoSeleccionado = rangoWin.getSeleccion();
        
        // Parsear rango
        double calMin = 0, calMax = 10;
        if (rangoSeleccionado == "1-2") { calMin = 1; calMax = 2; }
        else if (rangoSeleccionado == "3-4") { calMin = 3; calMax = 4; }
        else if (rangoSeleccionado == "5-6") { calMin = 5; calMax = 6; }
        else if (rangoSeleccionado == "7-8") { calMin = 7; calMax = 8; }
        else if (rangoSeleccionado == "9-10") { calMin = 9; calMax = 10; }
        
        // Filtrar películas
        std::vector<std::shared_ptr<Video>> videosFiltrados;
        std::ostringstream oss;
        oss << "Películas con calificación entre " << calMin << " y " << calMax << ":\n\n";
        
        for (const auto& video : catalogo) {
            if (video->getTipo() == "Pelicula" && 
                video->getCalificacion() >= calMin && 
                video->getCalificacion() <= calMax) {
                videosFiltrados.push_back(video);
                oss << video->getInfo() << "\n\n";
            }
        }
        
        actualizarPortadas(videosFiltrados);
        textBuffer->text(oss.str().c_str());
    }
    
    void calificarVideo() {
        // Obtener títulos
        std::vector<std::string> titulos;
        for (const auto& video : catalogo) {
            titulos.push_back(video->getTitulo());
        }
        
        // Ventana para seleccionar video
        SelectorWindow tituloWin("Seleccionar Video para Calificar", titulos);
        tituloWin.show();
        while (tituloWin.shown()) Fl::wait();
        if (tituloWin.fueCancelado()) return;
        std::string tituloSeleccionado = tituloWin.getSeleccion();
        
        // Solicitar calificación
        const char* input = fl_input("Ingresa la calificación (1-10):", "5");
        if (input) {
            try {
                int calificacion = std::stoi(input);
                if (calificacion >= 1 && calificacion <= 10) {
                    for (auto& video : catalogo) {
                        if (video->getTitulo() == tituloSeleccionado) {
                            video->actualizarCalificacion(calificacion);
                            std::ostringstream oss;
                            oss << "Nueva calificación para " << video->getTitulo() 
                                << ":\n" << std::fixed << std::setprecision(1) 
                                << video->getCalificacion();
                            fl_message("%s", oss.str().c_str());
                            actualizarPortadas();
                            return;
                        }
                    }
                    fl_alert("No se encontro el video");
                } else {
                    fl_alert("La calificación debe estar entre 1 y 10");
                }
            } catch (const std::exception& e) {
                fl_alert("Entrada invalida");
            }
        }
    }
};

int main() {
    fl_register_images();
    CatalogoApp app;
    app.run();
    return 0;
}