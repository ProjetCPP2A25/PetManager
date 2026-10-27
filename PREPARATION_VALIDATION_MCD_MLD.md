# Préparation de la validation – MCD et MLD (partie Stock)

## 1. MCD : ma partie (entité Stock)

**Entité : Stock** (un produit du stock du centre vétérinaire)

| Propriété | Rôle |
|---|---|
| **IdProduit** (souligné) | **Identifiant** : chaque produit est unique |
| Nom | nom du produit (unique) |
| Categorie | Alimentation, Médicament ou Équipement |
| Quantite | quantité disponible |
| SeuilMinimal | quantité minimale avant réapprovisionnement |
| PrixUnitaire | prix d'un article |
| DateExpiration | date limite d'utilisation |
| Emplacement | lieu de rangement |
| Statut | Disponible, Stock bas, Rupture ou Expiré |

**Association : Contenir** (entre Commande et Stock), porteuse des propriétés **Quantite** et **PrixUnitaire**.

| Côté | Cardinalité | Lecture |
|---|---|---|
| Commande | **(1,n)** | une commande contient **au moins un** produit et peut en contenir plusieurs |
| Stock | **(0,n)** | un produit peut n'apparaître dans **aucune** commande, ou dans plusieurs |

→ C'est une association **plusieurs-à-plusieurs** (n,n). Les propriétés Quantite et PrixUnitaire
ne dépendent ni de la commande seule ni du produit seul, mais du **couple** (commande, produit) :
elles sont donc placées **sur l'association**.

**Liens indirects** : le fournisseur d'un produit est connu par la commande
(Stock — Contenir — Commande — Passer — Fournisseur).

**(Si la table Consommation est validée par la tutrice)** : association **Enregistrer** entre Stock (0,n)
et Consommation (1,1). Un produit peut avoir zéro ou plusieurs consommations, et chaque consommation
concerne exactement un produit.

## 2. MLD : passage du MCD au modèle relationnel

| Règle de passage | Application à ma partie |
|---|---|
| Une entité devient une table ; l'identifiant devient la clé primaire | **Stock (<u>IdProduit</u>, Nom, Categorie, Quantite, SeuilMinimal, PrixUnitaire, DateExpiration, Emplacement, Statut)** |
| Une association (n,n) devient une table dont la clé primaire est composée des clés étrangères des deux entités ; les propriétés de l'association y sont placées | **Ligne_Cmd (<u>#IdCommande, #IdProduit</u>, Quantite, PrixUnitaire)** |
| Une association (1,1)–(0,n) : la clé de l'entité côté (0,n) migre comme clé étrangère côté (1,1) | **Consommation (<u>IdConso</u>, #IdProduit, Quantite, DateConso)** |

## 3. Contraintes d'intégrité (dans `base_commune_petmanager.sql`)

| Type | Contrainte | Exemple dans ma partie |
|---|---|---|
| **Intégrité d'entité** | clé primaire : unique et non nulle | `PK_STOCK (IDPRODUIT)`, `PK_LIGNE_CMD (IDCOMMANDE, IDPRODUIT)` |
| **Intégrité référentielle** | une clé étrangère doit exister dans la table référencée | `FK_LIGNE_STOCK` : LIGNE_CMD.IDPRODUIT → STOCK.IDPRODUIT ; `FK_LIGNE_COMMANDE` (ON DELETE CASCADE) ; `FK_CONSO_STOCK` |
| **Intégrité de domaine** | valeurs autorisées | `CK_STOCK_QUANTITE` (≥ 0), `CK_STOCK_SEUIL` (≥ 0), `CK_STOCK_PRIX` (> 0), `CK_STOCK_CATEGORIE`, `CK_STOCK_STATUT`, `CK_LIGNE_QUANTITE` (> 0) |
| **Valeurs obligatoires** | NOT NULL | Nom, Categorie, Quantite, SeuilMinimal, PrixUnitaire, DateExpiration, Emplacement |
| **Unicité** | UNIQUE | `UQ_STOCK_NOM` : deux produits ne peuvent pas avoir le même nom |

**Conséquence visible dans l'application** : un produit présent dans une commande ne peut pas être supprimé
(intégrité référentielle, erreur ORA-02292) ; l'application affiche alors un message clair.

## 4. Questions possibles et réponses

- **Pourquoi Stock n'a-t-il pas de clé étrangère ?** Parce que toutes ses associations sont du côté (0,n) :
  c'est la table Ligne_Cmd qui porte la clé étrangère #IdProduit.
- **Pourquoi la clé de Ligne_Cmd est-elle composée ?** Parce qu'elle vient d'une association (n,n) : le couple
  (IdCommande, IdProduit) identifie une ligne, et un même produit n'apparaît qu'une fois par commande.
- **Pourquoi Quantite et PrixUnitaire sont-ils dans Ligne_Cmd et aussi dans Stock ?** Dans Stock, c'est la
  quantité **en stock** et le prix **actuel** ; dans Ligne_Cmd, c'est la quantité **commandée** et le prix
  **au moment de la commande** (il peut changer ensuite).
- **Que signifie (0,n) côté Stock ?** Un produit peut exister sans avoir jamais été commandé.
- **Que signifie (1,n) côté Commande ?** Une commande vide n'a pas de sens : elle contient au moins un produit.
