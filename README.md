# Pet_manager

Application C++/Qt de gestion vétérinaire (équipe PetTech – ESPRIT).

Modules :

- Gestion du stock (Chahed Matoussi)
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
