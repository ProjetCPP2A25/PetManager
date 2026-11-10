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

### Généalogie, consanguinité et alertes (module Animaux)

- Bouton « Arbre généalogique » dans la fiche animal : arbre sur 4 générations (QGraphicsView), taux de consanguinité
  (coefficient de Wright, algorithme de graphe en C++ dans `animal.cpp`, classe `Pedigree`) et simulation d'accouplement.
- Bouton « Alertes » : catégories d'âge / bientôt senior, poids anormal par rapport à la race, anniversaires du mois.
- **Base de données** : la table ANIMAL reçoit deux colonnes `IDPERE` et `IDMERE`. Exécuter une fois `sql/ajout_genealogie.sql`
  (SYSTEM, F5) ; à relancer après chaque exécution de `sql/base_commune_petmanager.sql` (qui recrée les tables).
