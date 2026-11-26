#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Application console de gestion d'un club de lecture (C++17).
// Les fichiers texte sont des donnees separees par le caractere '|'.

class Personne {
protected:
    int id_;
    std::string nom_;
    std::string email_;
public:
    Personne(int id, std::string nom, std::string email)
        : id_(id), nom_(std::move(nom)), email_(std::move(email)) {}
    virtual ~Personne() = default;
    int id() const { return id_; }
    const std::string& nom() const { return nom_; }
    const std::string& email() const { return email_; }
    virtual std::string role() const = 0;
    virtual int limiteEmprunts() const = 0;
    virtual void afficher() const {
        std::cout << std::left << std::setw(6) << id_ << std::setw(26) << nom_
                  << std::setw(32) << email_ << role() << '\n';
    }
};

class Adherent : public Personne {
public:
    using Personne::Personne;
    std::string role() const override { return "Adherent"; }
    int limiteEmprunts() const override { return 3; }
};

class Bibliothecaire : public Personne {
public:
    using Personne::Personne;
    std::string role() const override { return "Bibliothecaire"; }
    int limiteEmprunts() const override { return 0; }
};

struct Livre {
    int id{};
    std::string titre, auteur, genre;
    int annee{};
    int exemplaires{};
};

struct Emprunt {
    int id{}, livreId{}, adherentId{};
    std::string dateEmprunt, dateRetour;
    bool actif{true};
};

class ClubLecture {
    std::vector<Livre> livres_;
    std::vector<std::unique_ptr<Personne>> personnes_;
    std::vector<Emprunt> emprunts_;
    const std::string fichierLivres_ = "livres.txt";
    const std::string fichierPersonnes_ = "personnes.txt";
    const std::string fichierEmprunts_ = "emprunts.txt";

    static std::vector<std::string> decouper(const std::string& ligne) {
        std::vector<std::string> champs;
        std::stringstream flux(ligne);
        std::string champ;
        while (std::getline(flux, champ, '|')) champs.push_back(champ);
        return champs;
    }
    static int entier(const std::string& s, int defaut = 0) {
        try { std::size_t n; int v = std::stoi(s, &n); return n == s.size() ? v : defaut; }
        catch (...) { return defaut; }
    }
    static std::string propre(std::string s) {
        std::replace(s.begin(), s.end(), '|', '/');
        std::replace(s.begin(), s.end(), '\n', ' ');
        return s;
    }
    int prochainLivre() const {
        int id = 1; for (const auto& x : livres_) id = std::max(id, x.id + 1); return id;
    }
    int prochainPersonne() const {
        int id = 1; for (const auto& x : personnes_) id = std::max(id, x->id() + 1); return id;
    }
    int prochainEmprunt() const {
        int id = 1; for (const auto& x : emprunts_) id = std::max(id, x.id + 1); return id;
    }
    Livre* trouverLivre(int id) {
        for (auto& x : livres_) if (x.id == id) return &x;
        return nullptr;
    }
    Personne* trouverPersonne(int id) {
        for (auto& x : personnes_) if (x->id() == id) return x.get();
        return nullptr;
    }
    int empruntsActifs(int id) const {
        return static_cast<int>(std::count_if(emprunts_.begin(), emprunts_.end(),
            [id](const Emprunt& e) { return e.adherentId == id && e.actif; }));
    }
    void charger() {
        std::ifstream f(fichierLivres_); std::string l;
        while (std::getline(f, l)) { auto c = decouper(l); if (c.size() == 6)
            livres_.push_back({entier(c[0]), c[1], c[2], c[3], entier(c[4]), entier(c[5])}); }
        std::ifstream p(fichierPersonnes_);
        while (std::getline(p, l)) { auto c = decouper(l); if (c.size() == 4) {
            int id = entier(c[0]);
            if (c[1] == "Bibliothecaire") personnes_.push_back(std::make_unique<Bibliothecaire>(id,c[2],c[3]));
            else if (c[1] == "Adherent") personnes_.push_back(std::make_unique<Adherent>(id,c[2],c[3]));
        }}
        std::ifstream e(fichierEmprunts_);
        while (std::getline(e, l)) { auto c = decouper(l); if (c.size() == 6)
            emprunts_.push_back({entier(c[0]), entier(c[1]), entier(c[2]), c[3], c[4], c[5] == "1"}); }
    }
    void sauvegarder() const {
        std::ofstream f(fichierLivres_);
        for (const auto& x : livres_) f << x.id << '|' << propre(x.titre) << '|' << propre(x.auteur)
            << '|' << propre(x.genre) << '|' << x.annee << '|' << x.exemplaires << '\n';
        std::ofstream p(fichierPersonnes_);
        for (const auto& x : personnes_) p << x->id() << '|' << x->role() << '|'
            << propre(x->nom()) << '|' << propre(x->email()) << '\n';
        std::ofstream e(fichierEmprunts_);
        for (const auto& x : emprunts_) e << x.id << '|' << x.livreId << '|' << x.adherentId
            << '|' << propre(x.dateEmprunt) << '|' << propre(x.dateRetour) << '|' << (x.actif ? 1 : 0) << '\n';
    }
    static std::string lireTexte(const std::string& invite) {
        std::cout << invite;
        std::string s; std::getline(std::cin, s); return propre(s);
    }
    static int lireEntier(const std::string& invite, int min = 0) {
        while (true) {
            std::cout << invite; std::string s; std::getline(std::cin, s);
            try { std::size_t n; int v = std::stoi(s, &n); if (n == s.size() && v >= min) return v; }
            catch (...) {}
            std::cout << "Saisie invalide. Entrez un nombre entier >= " << min << ".\n";
        }
    }
    void afficherLivres() const {
        std::cout << "\nID    Titre                         Auteur                        Genre             Annee  Dispo\n";
        for (const auto& x : livres_) std::cout << std::left << std::setw(6) << x.id
            << std::setw(30) << x.titre.substr(0,29) << std::setw(30) << x.auteur.substr(0,29)
            << std::setw(18) << x.genre.substr(0,17) << std::setw(7) << x.annee << x.exemplaires << '\n';
        if (livres_.empty()) std::cout << "Aucun livre enregistre.\n";
    }
    void afficherPersonnes() const {
        std::cout << "\nID    Nom                       Email                           Role\n";
        // Appel virtuel via Personne*: role()/afficher() depend du type reel.
        for (const auto& p : personnes_) p->afficher();
        if (personnes_.empty()) std::cout << "Aucun membre enregistre.\n";
    }
    void afficherEmprunts(bool actifsSeulement) const {
        std::cout << "\nID    Livre                       Adherent                    Emprunt       Retour        Etat\n";
        bool trouve = false;
        for (const auto& e : emprunts_) {
            if (actifsSeulement && !e.actif) continue;
            const Livre* livre = nullptr; const Personne* personne = nullptr;
            for (const auto& x : livres_) if (x.id == e.livreId) livre = &x;
            for (const auto& x : personnes_) if (x->id() == e.adherentId) personne = x.get();
            std::cout << std::left << std::setw(6) << e.id << std::setw(28) << (livre ? livre->titre : "[livre supprime]")
                << std::setw(28) << (personne ? personne->nom() : "[adherent supprime]")
                << std::setw(14) << e.dateEmprunt << std::setw(14) << (e.dateRetour.empty() ? "-" : e.dateRetour)
                << (e.actif ? "En cours" : "Rendu") << '\n'; trouve = true;
        }
        if (!trouve) std::cout << "Aucun emprunt a afficher.\n";
    }
    void ajouterLivre() {
        Livre x; x.id = prochainLivre(); x.titre = lireTexte("Titre : "); x.auteur = lireTexte("Auteur : ");
        x.genre = lireTexte("Genre : "); x.annee = lireEntier("Annee de publication : ", 0);
        x.exemplaires = lireEntier("Nombre d'exemplaires : ", 1);
        if (x.titre.empty() || x.auteur.empty()) { std::cout << "Titre et auteur sont obligatoires.\n"; return; }
        livres_.push_back(std::move(x)); sauvegarder(); std::cout << "Livre ajoute.\n";
    }
    void ajouterPersonne() {
        std::cout << "1. Adherent\n2. Bibliothecaire\n"; int choix = lireEntier("Type : ", 1);
        if (choix > 2) { std::cout << "Choix invalide.\n"; return; }
        std::string nom = lireTexte("Nom complet : "), email = lireTexte("Email : ");
        if (nom.empty() || email.empty()) { std::cout << "Nom et email sont obligatoires.\n"; return; }
        int id = prochainPersonne();
        if (choix == 1) personnes_.push_back(std::make_unique<Adherent>(id, nom, email));
        else personnes_.push_back(std::make_unique<Bibliothecaire>(id, nom, email));
        sauvegarder(); std::cout << "Inscription enregistree. Identifiant : " << id << '\n';
    }
    void emprunter() {
        afficherLivres(); int livreId = lireEntier("ID du livre : ", 1);
        Livre* l = trouverLivre(livreId);
        if (!l) { std::cout << "Livre introuvable.\n"; return; }
        int personneId = lireEntier("ID de l'adherent : ", 1); Personne* p = trouverPersonne(personneId);
        if (!p || p->role() != "Adherent") { std::cout << "Adherent introuvable.\n"; return; }
        if (l->exemplaires <= 0) { std::cout << "Refus : aucun exemplaire disponible.\n"; return; }
        // Polymorphisme: la limite depend de l'objet concret, interroge via Personne*.
        if (empruntsActifs(p->id()) >= p->limiteEmprunts()) {
            std::cout << "Refus : limite de " << p->limiteEmprunts() << " emprunts atteinte.\n"; return;
        }
        std::string date = lireTexte("Date d'emprunt (JJ/MM/AAAA) : ");
        emprunts_.push_back({prochainEmprunt(), livreId, personneId, date, "", true});
        --l->exemplaires; sauvegarder(); std::cout << "Emprunt enregistre.\n";
    }
    void retourner() {
        afficherEmprunts(true); int id = lireEntier("ID de l'emprunt a cloturer : ", 1);
        for (auto& e : emprunts_) if (e.id == id && e.actif) {
            std::string date = lireTexte("Date de retour (JJ/MM/AAAA) : ");
            e.actif = false; e.dateRetour = date;
            if (Livre* l = trouverLivre(e.livreId)) ++l->exemplaires;
            sauvegarder(); std::cout << "Retour enregistre.\n"; return;
        }
        std::cout << "Emprunt actif introuvable.\n";
    }
    void menuLivres() {
        while (true) { std::cout << "\n--- Gestion des livres ---\n1. Lister\n2. Ajouter\n0. Retour\n";
            int c = lireEntier("Choix : ", 0); if (c == 0) return; if (c == 1) afficherLivres(); else if (c == 2) ajouterLivre(); else std::cout << "Choix invalide.\n"; }
    }
    void menuPersonnes() {
        while (true) { std::cout << "\n--- Gestion des membres ---\n1. Lister\n2. Inscrire\n0. Retour\n";
            int c = lireEntier("Choix : ", 0); if (c == 0) return; if (c == 1) afficherPersonnes(); else if (c == 2) ajouterPersonne(); else std::cout << "Choix invalide.\n"; }
    }
public:
    ClubLecture() { charger(); }
    void executer() {
        std::cout << "====================================\n      CLUB DE LECTURE - GESTION\n====================================\n";
        while (true) {
            std::cout << "\nMenu principal\n1. Gerer les livres\n2. Gerer les membres\n3. Enregistrer un emprunt\n4. Enregistrer un retour\n5. Voir les emprunts en cours\n6. Historique des emprunts\n0. Quitter\n";
            int c = lireEntier("Votre choix : ", 0);
            switch (c) {
                case 0: sauvegarder(); std::cout << "Donnees sauvegardees. Au revoir !\n"; return;
                case 1: menuLivres(); break; case 2: menuPersonnes(); break;
                case 3: emprunter(); break; case 4: retourner(); break;
                case 5: afficherEmprunts(true); break; case 6: afficherEmprunts(false); break;
                default: std::cout << "Choix invalide.\n";
            }
        }
    }
};

int main() {
    ClubLecture club;
    club.executer();
    return 0;
}

