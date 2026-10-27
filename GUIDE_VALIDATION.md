# Guide de validation – Module Gestion de stock (Pet Manager)

## 1. Mise en place (à faire une seule fois)

1. **Base commune** : dans SQL Developer, connectée avec votre utilisateur, exécuter
   `sql/base_commune_petmanager.sql` (bouton « Exécuter le script »). Il crée les 7 tables du MR
   (ANIMAL, EMPLOYE, RENDEZVOUS, FOURNISSEUR, STOCK, COMMANDE, LIGNE_CMD) + CONSOMMATION,
   avec toutes les PK, FK et contraintes. Les accents sont écrits avec `UNISTR`, donc ils restent
   corrects quel que soit l'encodage de SQL Developer.
2. **Données de test** : exécuter `sql/donnees_test.sql` (fournisseurs, employés, animaux,
   rendez-vous, 15 produits, 5 commandes, 30 jours de consommation). Il peut être relancé.
3. **Connexion** : `connection.cpp` contient la source `Source_Projet2A`, l'utilisateur `chahed`
   et le mot de passe `esprit18` (méthode `createInstance()` comme dans le code de l'atelier).
4. **Qt Creator** : ouvrir `Pet_manager.pro`, puis *Compiler > Exécuter qmake*, puis *Recompiler*.

**Fournisseur d'un produit** : conformément au MR commun, la table STOCK n'a pas de clé étrangère
vers FOURNISSEUR. Le fournisseur d'un produit est retrouvé par ses commandes
(STOCK → LIGNE_CMD → COMMANDE → FOURNISSEUR, dernière commande) dans `Produit::fournisseurParProduit()`.
Il apparaît dans la fiche de commande Excel, l'e-mail d'alerte et la page Alertes.

## 2. Correspondance avec la partie « C. Implémentation des CRUDs »

| Étape demandée | Où dans le projet |
|---|---|
| Créer la classe C++ de l'entité dans un `.h` | `produit.h` : classe `Produit` (attributs = colonnes de STOCK, constructeurs, getters/setters) |
| Implémenter les méthodes dans le `.cpp` | `produit.cpp` |
| Connecter la classe à Oracle | `connection.h/.cpp` (Singleton, source ODBC `Source_Projet2A`) + `main.cpp` (connexion **avant** la création de la fenêtre) |
| Ajouter (Create) | `Produit::ajouter()` (INSERT préparé) ← appelé par `on_btnEnregistrerStock_clicked()` |
| Afficher (Read) | `Produit::afficher()` retourne un `QSqlQueryModel*` ← `ui->tableStock->setModel(Etmp.afficher())` (constructeur) puis actualisation après chaque modification |
| Modifier (Update) | `Produit::modifier()` (UPDATE préparé) ← `on_btnModifierStock_clicked()` ; aussi `retirerQuantite()` (sortie de stock) |
| Supprimer (Delete) | `Produit::supprimer(int)` (DELETE préparé) ← `on_btnSupprimerStock_clicked()` via l'attribut `Etmp` |
| Tester / corriger | Scénarios du paragraphe 4 ci-dessous |

Règles des ateliers respectées :
- **aucune requête derrière un bouton** : `gpetmanager.cpp` ne contient aucun SQL ;
- **requêtes préparées** partout (`prepare()` + `bindValue()` + `exec()`) → protection contre l'injection SQL
  (le tri est choisi dans une liste fixe, jamais à partir du texte saisi) ;
- **une seule connexion** (Singleton) ;
- **contrôle de saisie en C++** : `Produit::controleSaisie()` + validateurs sur les champs + ID unique,
  nom unique, date future, emplacement valide ;
- **contraintes dans la BD** : PK, NOT NULL, UNIQUE, CHECK (quantité, seuil, prix, catégorie, statut) ;
- **un seul `.ui`**, navigation par `QStackedWidget`, aucune fenêtre `QDialog` supplémentaire.

## 3. Métiers basiques du cahier de spécification

| Métier | Où dans le projet |
|---|---|
| 1. Rechercher un produit selon son nom | Champ de recherche au-dessus du tableau → `Produit::afficher(rechercheNom, ...)` (`WHERE UPPER(NOM) LIKE UPPER(:recherche)`, requête préparée) |
| 2. Trier les produits selon la date d'expiration | Liste « Aucun tri / la plus proche / la plus lointaine » → `ORDER BY DATEEXPIRATION ASC/DESC` dans `Produit::afficher()` |
| 3. Fiche de commande de réapprovisionnement en Excel | Bouton « Fiche Excel » (page Stock) ou « Fiche de commande » (page Alertes) → `Produit::genererFicheCommande()` : vrai fichier **.xlsx** avec N° de commande, date, livraison souhaitée, produits regroupés **par fournisseur**, stock, seuil, **quantité à commander**, prix, montants, sous-totaux, total général et zone de signature. Prêt à imprimer (paysage A4) ou à envoyer au fournisseur. |
| 4. Graphique de l'évolution de la consommation et répartition | Courbe jour par jour (7 ou 30 jours) des quantités consommées, pour les 3 produits les plus consommés ou pour un produit choisi (table CONSOMMATION, alimentée à chaque sortie de stock). L'infobulle indique le total, la moyenne et le **pic** de consommation. Camembert : répartition des produits par catégorie. |

**Remarque BD :** la courbe de consommation a besoin d'un historique ; la table `CONSOMMATION`
(IDCONSO, #IDPRODUIT, QUANTITE, DATECONSO) est donc ajoutée au modèle relationnel et physique
(table d'historique liée à STOCK par la clé étrangère #IDPRODUIT). Pensez à l'ajouter au MR / MPD de l'équipe.

## 4. Les deux métiers innovants

### Métier 1 – Alerte automatique de réapprovisionnement par e-mail
- Surveillance automatique : au démarrage, après chaque ajout / modification / sortie de stock,
  puis toutes les *N* minutes (réglable en haut de la page « Alertes »).
- Un produit est signalé dès que `QUANTITE <= SEUILMINIMAL` (requête `Produit::listeSousSeuil()`).
- L'e-mail (HTML) envoyé au responsable contient : produit, ID, catégorie, emplacement,
  quantité restante, seuil, **quantité recommandée** (= stock cible × seuil − quantité),
  fournisseur et montant estimé.
- Un produit n'est envoyé qu'une fois tant qu'il reste sous le seuil (pas de spam) ;
  bouton « Envoyer l'alerte e-mail » pour un envoi manuel ; historique des envois.

Configuration (page Stock > carte « Alerte e-mail » > Ouvrir > « Paramètres des métiers ») :
- Serveur `smtp.gmail.com`, port `465`.
- Expéditeur : votre adresse Gmail. Mot de passe : un **mot de passe d'application**
  (compte Google > Sécurité > Validation en deux étapes activée > Mots de passe des applications).
  Le mot de passe normal du compte ne fonctionne pas.
- E-mail du responsable : l'adresse qui reçoit les alertes (plusieurs possibles, séparées par `;`).
- Cliquer **Tester l'e-mail**, puis **Enregistrer**.

### Métier 2 – Suggestion intelligente de produits alternatifs (IA)
- Déclenchement : recherche d'un besoin dans le panneau « Alternatives suggérées par l'IA »,
  bouton IA d'une ligne du tableau, sortie de stock d'un produit en rupture / expiré / insuffisant,
  ou recherche sans résultat (lien « demander une alternative à l'IA »).
- L'IA (Google Gemini) reçoit le besoin et le catalogue des produits **réellement disponibles**
  (lu dans STOCK), analyse la fonction et l'usage, et renvoie au maximum 3 alternatives
  (même d'une autre catégorie) avec la pertinence, la raison et les avantages/précautions.
- Les ID renvoyés sont vérifiés : l'IA ne peut pas inventer un produit.
- L'utilisateur garde la décision : bouton **Utiliser** (sortie de stock de l'alternative) ou **Voir**.
- Sans clé API ou sans Internet, un moteur local (mots-clés + catégorie) prend le relais et un
  message l'indique.

Configuration : créer une clé gratuite sur <https://aistudio.google.com> (« Get API key »),
la coller dans « Clé API », laisser le modèle `gemini-flash-latest` (ou un autre modèle Gemini
disponible), puis **Enregistrer**.

Les paramètres sont stockés dans `petmanager.ini`, à côté de l'exécutable (dossier `build/...`).
Ce fichier contient le mot de passe d'application et la clé API : ne pas le pousser sur GitHub.

## 5. Scénarios de test (à montrer en validation)

| # | Test | Résultat attendu |
|---|---|---|
| 1 | Lancer l'application | Message « Connexion réussie », tableau rempli depuis Oracle |
| 2 | Ajouter un produit valide (ID proposé, nom, catégorie, quantité 20, seuil 5, prix, date future, emplacement, fournisseur) | « Ajout effectué », la ligne apparaît sans redémarrer |
| 3 | Ajouter avec un ID déjà utilisé, un nom vide, un prix à 0 ou une date passée | Message « Contrôle de saisie » listant les erreurs, rien n'est inséré |
| 4 | Rechercher `amoxi`, puis choisir « Date d'expiration : la plus proche d'abord » | Le tableau est filtré par nom et trié par date |
| 5 | Crayon sur une ligne → changer la quantité → Modifier | « Modification effectuée », statut recalculé |
| 6 | Supprimer (corbeille ou bouton Supprimer) | Confirmation puis « Suppression effectuée » |
| 7 | Sortie de stock qui fait passer un produit sous son seuil | Toast puis e-mail reçu par le responsable, ligne dans l'historique |
| 8 | Sortie de stock sur « Vaccin CHPP » (rupture) | L'IA propose « Vaccin CHPPi-L » avec la justification |
| 9 | Panneau IA : saisir « vermifuge chat » | Alternatives (ex. pipette antiparasitaire) car le vermifuge est en rupture |
| 10 | Bouton « Fiche Excel » | Fichier `Fiche_commande_CMD-....xlsx` regroupé par fournisseur, ouvrable dans Excel |
| 11 | Injection SQL : rechercher `' OR '1'='1` | Aucun résultat : le texte est traité comme une valeur, pas comme du SQL |
| 12 | Graphique : choisir un produit, puis 7 / 30 jours | La courbe change ; après une sortie de stock, le point du jour augmente |
