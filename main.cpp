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

// Clase base Video (para polimorfismo)
class Video {
protected:
    std::string titulo;
    double calificacion;
    std::string genero;
    std::string director;
    int anio;
    std::string rutaPortada;

public:
    Video(const std::string& t, double cal, const std::string& g, 
          const std::string& dir, int a, const std::string& rp)
        : titulo(t), calificacion(cal), genero(g), director(dir), 
          anio(a), rutaPortada(rp) {}
    
    virtual ~Video() = default;
    
    // Métodos virtuales para polimorfismo
    virtual std::string getTipo() const = 0;
    virtual std::string getInfo() const = 0;
    virtual std::string getRutaVideo() const = 0;
    
    // Getters
    std::string getTitulo() const { return titulo; }
    std::string getGenero() const { return genero; }
    std::string getRutaPortada() const { return rutaPortada; }
    double getCalificacion() const { return calificacion; }
    std::string getDirector() const { return director; }
    int getAnio() const { return anio; }
    
    // Método para actualizar calificación
    void actualizarCalificacion(int nuevaCalificacion) {
        calificacion = (calificacion + nuevaCalificacion) / 2;
    }
    
    void setCalificacion(double cal) {
        calificacion = cal;
    }
};

// Clase derivada Pelicula
class Pelicula : public Video {
private:
    int duracion;
    
public:
    Pelicula(const std::string& t, double cal, int d, const std::string& g, 
             const std::string& dir, int a, const std::string& rp)
        : Video(t, cal, g, dir, a, rp), duracion(d) {}
    
    std::string getTipo() const override { return "Pelicula"; }
    
    std::string getInfo() const override {
        std::ostringstream oss;
        oss << "Película: " << titulo << " | Género: " << genero 
            << " | Duración: " << duracion << " min | Director: " << director
            << " | Año: " << anio << " | Calificación: " << std::fixed << std::setprecision(1) << calificacion;
        return oss.str();
    }
    
    std::string getRutaVideo() const override {
        return "videos/" + titulo + ".mp4"; // Ruta ficticia, deberías ajustarla
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
          int nt, int te, const std::string& dir, const std::string& rp)
        : Video(t, cal, g, dir, 0, rp), episodiosPorTemporada(ept), 
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
        return "series/" + titulo + "_S01E01.mp4"; // Ruta ficticia, deberías ajustarla
    }
    
    int getEpisodiosPorTemporada() const { return episodiosPorTemporada; }
    int getNumTemporadas() const { return numTemporadas; }
    int getTotalEpisodios() const { return totalEpisodios; }
};

// Widget personalizado para portadas clickeables
class PortadaBox : public Fl_Box {
private:
    std::shared_ptr<Video> video;
    Fl_Image* imagen;
    static void* callback_data;

public:
    PortadaBox(int x, int y, std::shared_ptr<Video> v) 
        : Fl_Box(x, y, 120, 160), video(v), imagen(nullptr) {
        
        box(FL_BORDER_BOX);
        color(FL_BLACK);
        labelcolor(FL_WHITE);
        
        // Cargar imagen de portada
        if (!video->getRutaPortada().empty()) {
            imagen = Fl_Shared_Image::get(video->getRutaPortada().c_str());
            if (imagen) {
                image(imagen);
            }
        }
        
        if (!imagen) {
            label("Sin\nImagen");
            align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
        }
        
        callback_data = this;
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
        std::string comando;
        std::string rutaVideo = video->getRutaVideo();
        
        // Detectar sistema operativo y usar reproductor apropiado
        #ifdef _WIN32
            comando = "start \"\" \"" + rutaVideo + "\"";
        #elif __APPLE__
            comando = "open \"" + rutaVideo + "\"";
        #else
            comando = "xdg-open \"" + rutaVideo + "\"";
        #endif
        
        system(comando.c_str());
        
        // Mostrar información
        fl_message("Reproduciendo:\n%s", video->getInfo().c_str());
    }
    
    std::shared_ptr<Video> getVideo() const { return video; }
};

void* PortadaBox::callback_data = nullptr;

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
        // Ventana principal
        window = new Fl_Window(1000, 700, "Catálogo de Películas y Series");
        window->color(FL_BLACK);
        
        // Menú superior
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
        
        // Filtros
        filtroInput = new Fl_Input(480, 20, 150, 30, "Filtro:");
        filtroInput->color(FL_DARK3);
        filtroInput->textcolor(FL_WHITE);
        
        calificacionSpinner = new Fl_Spinner(700, 20, 80, 30, "Cal. mín:");
        calificacionSpinner->minimum(0);
        calificacionSpinner->maximum(10);
        calificacionSpinner->value(0);
        calificacionSpinner->color(FL_DARK3);
        calificacionSpinner->textcolor(FL_WHITE);
        
        // Área de portadas
        scrollPortadas = new Fl_Scroll(20, 70, 960, 300);
        scrollPortadas->color(FL_BLACK);
        
        packPortadas = new Fl_Pack(25, 75, 950, 290);
        packPortadas->type(Fl_Pack::HORIZONTAL);
        packPortadas->spacing(10);
        
        scrollPortadas->end();
        
        // Área de resultados
        Fl_Box* labelResultados = new Fl_Box(20, 380, 100, 20, "Resultados:");
        labelResultados->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        labelResultados->labelcolor(FL_WHITE);
        
        resultadosDisplay = new Fl_Text_Display(20, 400, 960, 280);
        textBuffer = new Fl_Text_Buffer();
        resultadosDisplay->buffer(textBuffer);
        resultadosDisplay->color(FL_DARK3);
        resultadosDisplay->textcolor(FL_WHITE);
        
        window->end();
    }
    
    static void ejecutarCallback(Fl_Widget* w, void* data) {
        CatalogoApp* app = (CatalogoApp*)data;
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
                exit(0);
                break;
        }
    }
    
    void cargarDatosPorDefecto() {
        // Películas
        catalogo.push_back(std::make_shared<Pelicula>(
            "La princesa Mononoke", 8.4, 134, "Fantasía/Aventura", 
            "Hayao Miyazaki", 1997, "Poster con San y los espíritus del bosque"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "El viaje de Chihiro", 8.6, 125, "Fantasía/Aventura", 
            "Hayao Miyazaki", 2001, "Poster de Chihiro en el mundo de los espíritus"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "Look Back", 8.1, 90, "Drama/Slice of Life", 
            "Kiyotaka Oshiyama", 2021, "Poster de dos lesbianas"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars: Episodio I - La amenaza fantasma", 6.5, 136, "Ciencia ficción", 
            "George Lucas", 1999, "Poster con Darth Maul y Anakin"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars: Episodio II - El ataque de los clones", 6.5, 142, "Ciencia ficción", 
            "George Lucas", 2002, "Poster con Anakin y Padmé"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars: Episodio III - La venganza de los Sith", 7.5, 140, "Ciencia ficción", 
            "George Lucas", 2005, "Poster con Anakin y Obi-Wan luchando"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars: Episodio IV - Una nueva esperanza", 8.6, 121, "Ciencia ficción", 
            "George Lucas", 1977, "Poster clásico de Star Wars"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars: Episodio V - El imperio contraataca", 8.7, 124, "Ciencia ficción", 
            "Irvin Kershner", 1980, "Poster con Han Solo y Leia"));
            
        catalogo.push_back(std::make_shared<Pelicula>(
            "Star Wars: Episodio VI - El retorno del Jedi", 8.3, 131, "Ciencia ficción", 
            "Richard Marquand", 1983, "Poster con Luke y Darth Vader"));
            
        // Series
        catalogo.push_back(std::make_shared<Serie>(
            "Jujutsu Kaisen", 8.7, 24, "Acción/Fantasía oscura", 
            2, 47, "Sunghoo Park", "Poster con Yuji Itadori y Sukuna"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Pokémon", 7.5, 22, "Aventura/Fantasía", 
            25, 1200, "Kunihiko Yuyama", "Poster con Pikachu y Ash"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Violet Evergarden", 8.8, 24, "Drama/Fantasía", 
            1, 13, "Taichi Ishidate", "Poster con violet"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Kimetsu no Yaiba", 8.7, 24, "Acción/Fantasía oscura", 
            3, 55, "Haruo Sotozaki", "Poster con Tanjiro y Nezuko"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Attack on Titan", 9.0, 24, "Acción/Fantasía oscura", 
            4, 87, "Tetsurō Araki", "Poster con Eren y los titanes"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Blue Lock", 8.3, 24, "Deporte/Drama", 
            1, 24, "Tetsuaki Watanabe", "Poster con los jugadores de fútbol"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Star Wars: The Clone Wars", 8.4, 22, "Ciencia ficción", 
            7, 133, "Dave Filoni", "Poster con Anakin y Ahsoka"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Ann", 7.9, 45, "Drama/Thriller", 
            1, 10, "Unknown", "Poster de la niña pelirroja"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Nadie nos va a extrañar", 8.1, 45, "Drama/Crimen", 
            1, 10, "Unknown", "Niños de mexico noventeros"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Si la vida te da mandarinas", 7.8, 45, "Drama/Comedia", 
            1, 10, "Unknown", "un dulce k-drama"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Goblin", 8.9, 70, "Fantasía/Romance", 
            1, 16, "Lee Eung-bok", "Poster del mejor k.drama de la historia"));
            
        catalogo.push_back(std::make_shared<Serie>(
            "Alien Stage", 8.5, 15, "Ciencia ficción/Musical", 
            1, 6, "Unknown", "nose la sofi jsjsjs"));
    }
    
    void actualizarPortadas() {
        // Limpiar portadas anteriores
        for (auto* portada : portadas) {
            packPortadas->remove(portada);
            delete portada;
        }
        portadas.clear();
        
        // Crear nuevas portadas
        int x = 25;
        for (auto& video : catalogo) {
            PortadaBox* portada = new PortadaBox(x, 75, video);
            portadas.push_back(portada);
            packPortadas->add(portada);
            x += 130;
        }
        
        packPortadas->redraw();
        scrollPortadas->redraw();
    }
    
    void cargarArchivoDatos() {
        const char* filename = fl_file_chooser("Seleccionar archivo", "*.txt", "");
        if (filename) {
            std::string resultado = "Archivo cargado: ";
            resultado += filename;
            textBuffer->text(resultado.c_str());
        }
    }
    
    void mostrarVideosPorCalificacionOGenero() {
        std::string filtro = filtroInput->value() ? filtroInput->value() : "";
        double calMin = calificacionSpinner->value();
        
        std::string resultado = "Videos filtrados:\n\n";
        for (auto& video : catalogo) {
            bool coincideGenero = filtro.empty() || 
                video->getGenero().find(filtro) != std::string::npos ||
                video->getTitulo().find(filtro) != std::string::npos;
            bool coincideCalif = video->getCalificacion() >= calMin;
            
            if (coincideGenero && coincideCalif) {
                resultado += video->getInfo() + "\n\n";
            }
        }
        textBuffer->text(resultado.c_str());
    }
    
    void mostrarEpisodiosSerie() {
        std::string titulo = filtroInput->value() ? filtroInput->value() : "";
        double calMin = calificacionSpinner->value();
        
        std::string resultado = "Información de series:\n\n";
        for (auto& video : catalogo) {
            if (video->getTipo() == "Serie") {
                bool coincideTitulo = titulo.empty() || 
                    video->getTitulo().find(titulo) != std::string::npos;
                bool coincideCalif = video->getCalificacion() >= calMin;
                
                if (coincideTitulo && coincideCalif) {
                    auto serie = std::dynamic_pointer_cast<Serie>(video);
                    resultado += video->getInfo() + "\n\n";
                }
            }
        }
        textBuffer->text(resultado.c_str());
    }
    
    void mostrarPeliculasPorCalificacion() {
        double calMin = calificacionSpinner->value();
        
        std::ostringstream oss;
        oss << "Películas con calificación >= " << calMin << ":\n\n";
        
        for (auto& video : catalogo) {
            if (video->getTipo() == "Pelicula" && video->getCalificacion() >= calMin) {
                oss << video->getInfo() << "\n\n";
            }
        }
        textBuffer->text(oss.str().c_str());
    }
    
    void calificarVideo() {
        std::string titulo = filtroInput->value() ? filtroInput->value() : "";
        if (titulo.empty()) {
            fl_alert("Ingresa el título a calificar en el campo filtro");
            return;
        }
        
        const char* input = fl_input("Calificación (1-10):", "5");
        if (input) {
            int calificacion = std::stoi(input);
            if (calificacion >= 1 && calificacion <= 10) {
                for (auto& video : catalogo) {
                    if (video->getTitulo().find(titulo) != std::string::npos) {
                        video->actualizarCalificacion(calificacion);
                        
                        std::ostringstream oss;
                        oss << "Nueva calificación para " << video->getTitulo() 
                            << ":\n" << std::fixed << std::setprecision(1) 
                            << video->getCalificacion();
                        
                        fl_message("%s", oss.str().c_str());
                        return;
                    }
                }
                fl_alert("No se encontró el video con ese título");
            } else {
                fl_alert("La calificación debe estar entre 1 y 10");
            }
        }
    }
};

int main() {
    // Inicializar FLTK para imágenes
    fl_register_images();
    
    CatalogoApp app;
    app.run();
    
    return 0;
}