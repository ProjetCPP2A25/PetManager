# Pet_manager

Application C++/Qt de gestion vétérinaire (équipe PetTech – ESPRIT).

Modules :

- Gestion du stock (Chahed Matoussi)
- Gestion des animaux (CRUD Oracle + statistiques)
- Gestion des commandes
- Gestion des rendez-vous

## Module Gestion de stock

Architecture Modèle-Vue (atelier 3) :

| Couche  | Fichiers | Rôle |
|---------|----------|------|
| Connexion | `connection.h`, `connection.cpp` | Une seule connexion Oracle (patron Singleton, atelier 2) |
| Modèle  | `produit.h`, `produit.cpp` | **Seule classe du module** : entité Produit (table STOCK) avec le CRUD, les métiers basiques (recherche, tri, fiche Excel, statistiques) et les métiers innovants (alerte e-mail, alternatives IA). Requêtes toutes préparées |
| Vue     | `gpetmanager.ui`, `gpetmanager.h`, `gpetmanager.cpp` | Interface, affichage, graphiques, interactions |

Scripts : `sql/base_commune_petmanager.sql` (base commune) et `sql/donnees_test.sql`. Guide complet de test : `GUIDE_VALIDATION.md`.

## Module Animaux

Intégré dans la page « Animaux » du menu de gauche (`GPetManager`, section `MODULE ANIMAUX : initialisation`).
Ses fichiers sont ceux de son projet, sans modification : `animal.h/.cpp` (entité Animal avec CRUD, contrôle de saisie,
requêtes préparées, métiers, et widgets de graphiques), `connexion.h/.cpp`, `mainwindow.h/.cpp/.ui`.
La connexion Oracle est celle de `Connection` (ouverte dans `main()`) : `Connexion::ouvrir()` la réutilise.
La table ANIMAL contient la colonne `STATUT` (voir `sql/base_commune_petmanager.sql`). Données de test : `sql/donnees_animaux.sql`.

## Module Rendez-vous

Intégré dans la page « Rendez-vous » du menu de gauche (`GPetManager`, section `MODULE RENDEZ-VOUS : initialisation`).
Fichiers repris de son projet : `rendezvous.h/.cpp` (entité Rendezvous : CRUD, contrôle de saisie, recherche, filtre, tri)
et son interface, renommée `rendezvousvue.h/.cpp/.ui` (classe `GPetManager` → `RendezVousVue`, pour éviter le conflit
avec la fenêtre principale). Son menu latéral et son en-tête sont masqués : on utilise ceux de l'application.
La connexion Oracle est celle de `Connection` (ouverte dans `main()`).
À exécuter une fois : `sql/modif_rendezvous.sql` (IDEMPLOYE accepte NULL, le formulaire ne demande pas encore l'employé).
Icônes ajoutées : `icones/pen.png`, `icones/delete.png`.
