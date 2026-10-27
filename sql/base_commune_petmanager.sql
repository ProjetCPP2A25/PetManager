-- NOTE ENCODAGE : les textes accentués sont écrits avec UNISTR('\00E9' = é ...)
-- pour que les accents soient corrects quel que soit l'encodage de SQL Developer.
-- =====================================================================
--  PET MANAGER - Modèle physique de la base de données COMMUNE
--  Fidèle au MEA / MR de l'équipe :
--    Animal, Employe, RendezVous, Fournisseur, Stock, Commande, Ligne_Cmd
--  + Consommation (historique des sorties de stock, nécessaire au
--    graphique « évolution de la consommation » du cahier de spécification)
--
--  Oracle 11g XE - à exécuter par CHAQUE membre dans son utilisateur
--  (SQL Developer : bouton « Exécuter le script »)
-- =====================================================================

-- Pour tout recréer, décommenter ce bloc (ordre inverse des clés étrangères) :
-- DROP TABLE CONSOMMATION CASCADE CONSTRAINTS;
-- DROP TABLE LIGNE_CMD    CASCADE CONSTRAINTS;
-- DROP TABLE COMMANDE     CASCADE CONSTRAINTS;
-- DROP TABLE RENDEZVOUS   CASCADE CONSTRAINTS;
-- DROP TABLE STOCK        CASCADE CONSTRAINTS;
-- DROP TABLE FOURNISSEUR  CASCADE CONSTRAINTS;
-- DROP TABLE EMPLOYE      CASCADE CONSTRAINTS;
-- DROP TABLE ANIMAL       CASCADE CONSTRAINTS;
-- DROP SEQUENCE SEQ_CONSOMMATION;

-- ---------------------------------------------------------------------
--  ANIMAL (IdAnimal, Photo, Nom, Espece, Race, Sexe, DateNaissance, Poids, Description)
-- ---------------------------------------------------------------------
CREATE TABLE ANIMAL (
    IDANIMAL        NUMBER(8)       CONSTRAINT PK_ANIMAL PRIMARY KEY,
    PHOTO           VARCHAR2(255),
    NOM             VARCHAR2(50)    CONSTRAINT NN_ANIMAL_NOM NOT NULL,
    ESPECE          VARCHAR2(30)    CONSTRAINT NN_ANIMAL_ESPECE NOT NULL,
    RACE            VARCHAR2(50),
    SEXE            VARCHAR2(10),
    DATENAISSANCE   DATE,
    POIDS           NUMBER(6,2),
    DESCRIPTION     VARCHAR2(500),
    CONSTRAINT CK_ANIMAL_SEXE  CHECK (SEXE IN (UNISTR('M\00E2le'), 'Femelle')),
    CONSTRAINT CK_ANIMAL_POIDS CHECK (POIDS > 0)
);

-- ---------------------------------------------------------------------
--  EMPLOYE (IdEmploye, Nom, Prenom, Email, Telephone, Poste, Salaire, DateEmbauche, Statut)
-- ---------------------------------------------------------------------
CREATE TABLE EMPLOYE (
    IDEMPLOYE       NUMBER(8)       CONSTRAINT PK_EMPLOYE PRIMARY KEY,
    NOM             VARCHAR2(50)    CONSTRAINT NN_EMPLOYE_NOM NOT NULL,
    PRENOM          VARCHAR2(50)    CONSTRAINT NN_EMPLOYE_PRENOM NOT NULL,
    EMAIL           VARCHAR2(100),
    TELEPHONE       VARCHAR2(20),
    POSTE           VARCHAR2(50),
    SALAIRE         NUMBER(10,3),
    DATEEMBAUCHE    DATE,
    STATUT          VARCHAR2(20),
    CONSTRAINT UQ_EMPLOYE_EMAIL   UNIQUE (EMAIL),
    CONSTRAINT CK_EMPLOYE_SALAIRE CHECK (SALAIRE > 0)
);

-- ---------------------------------------------------------------------
--  FOURNISSEUR (IdFournisseur, Nom, Contact, Telephone, Email, Adresse,
--               TypeProduit, Statut, DateContrat)
-- ---------------------------------------------------------------------
CREATE TABLE FOURNISSEUR (
    IDFOURNISSEUR   NUMBER(8)       CONSTRAINT PK_FOURNISSEUR PRIMARY KEY,
    NOM             VARCHAR2(50)    CONSTRAINT NN_FOURNISSEUR_NOM NOT NULL,
    CONTACT         VARCHAR2(50),
    TELEPHONE       VARCHAR2(20),
    EMAIL           VARCHAR2(100),
    ADRESSE         VARCHAR2(150),
    TYPEPRODUIT     VARCHAR2(30),
    STATUT          VARCHAR2(20),
    DATECONTRAT     DATE
);

-- ---------------------------------------------------------------------
--  STOCK (IdProduit, Nom, Categorie, Quantite, SeuilMinimal, PrixUnitaire,
--         DateExpiration, Emplacement, Statut)
-- ---------------------------------------------------------------------
CREATE TABLE STOCK (
    IDPRODUIT       NUMBER(8)       CONSTRAINT PK_STOCK PRIMARY KEY,
    NOM             VARCHAR2(50)    CONSTRAINT NN_STOCK_NOM NOT NULL,
    CATEGORIE       VARCHAR2(30)    CONSTRAINT NN_STOCK_CATEGORIE NOT NULL,
    QUANTITE        NUMBER(8)       DEFAULT 0 CONSTRAINT NN_STOCK_QUANTITE NOT NULL,
    SEUILMINIMAL    NUMBER(8)       DEFAULT 0 CONSTRAINT NN_STOCK_SEUIL NOT NULL,
    PRIXUNITAIRE    NUMBER(10,3)    CONSTRAINT NN_STOCK_PRIX NOT NULL,
    DATEEXPIRATION  DATE            CONSTRAINT NN_STOCK_DATE NOT NULL,
    EMPLACEMENT     VARCHAR2(30)    CONSTRAINT NN_STOCK_EMPLACEMENT NOT NULL,
    STATUT          VARCHAR2(20)    DEFAULT 'Disponible',
    CONSTRAINT UQ_STOCK_NOM        UNIQUE (NOM),
    CONSTRAINT CK_STOCK_QUANTITE   CHECK (QUANTITE >= 0),
    CONSTRAINT CK_STOCK_SEUIL      CHECK (SEUILMINIMAL >= 0),
    CONSTRAINT CK_STOCK_PRIX       CHECK (PRIXUNITAIRE > 0),
    CONSTRAINT CK_STOCK_CATEGORIE  CHECK (CATEGORIE IN ('Alimentation', UNISTR('M\00E9dicament'), UNISTR('\00C9quipement'))),
    CONSTRAINT CK_STOCK_STATUT     CHECK (STATUT IN ('Disponible', 'Stock bas', 'Rupture', 'Expire'))
);

-- ---------------------------------------------------------------------
--  RENDEZVOUS (IdRdv, DateRdv, Heure, Statut, Motif, #IdAnimal, #IdEmploye)
--  Concerner : (1,1) côté RendezVous  -> #IdAnimal obligatoire
--  Assurer   : (1,1) côté RendezVous  -> #IdEmploye obligatoire
-- ---------------------------------------------------------------------
CREATE TABLE RENDEZVOUS (
    IDRDV           NUMBER(8)       CONSTRAINT PK_RENDEZVOUS PRIMARY KEY,
    DATERDV         DATE            CONSTRAINT NN_RDV_DATE NOT NULL,
    HEURE           VARCHAR2(5)     CONSTRAINT NN_RDV_HEURE NOT NULL,   -- format HH:MI
    STATUT          VARCHAR2(20),
    MOTIF           VARCHAR2(200),
    IDANIMAL        NUMBER(8)       CONSTRAINT NN_RDV_ANIMAL NOT NULL,
    IDEMPLOYE       NUMBER(8)       CONSTRAINT NN_RDV_EMPLOYE NOT NULL,
    CONSTRAINT FK_RDV_ANIMAL  FOREIGN KEY (IDANIMAL)  REFERENCES ANIMAL (IDANIMAL) ON DELETE CASCADE,
    CONSTRAINT FK_RDV_EMPLOYE FOREIGN KEY (IDEMPLOYE) REFERENCES EMPLOYE (IDEMPLOYE)
);

-- ---------------------------------------------------------------------
--  COMMANDE (IdCommande, DateCommande, DateLivraison, Statut, MontantTotal,
--            #IdFournisseur, #IdEmploye)
--  Passer    : (1,1) côté Commande -> #IdFournisseur obligatoire
--  Confirmer : (1,1) côté Commande -> #IdEmploye obligatoire
-- ---------------------------------------------------------------------
CREATE TABLE COMMANDE (
    IDCOMMANDE      NUMBER(8)       CONSTRAINT PK_COMMANDE PRIMARY KEY,
    DATECOMMANDE    DATE            DEFAULT SYSDATE CONSTRAINT NN_CMD_DATE NOT NULL,
    DATELIVRAISON   DATE,
    STATUT          VARCHAR2(20),
    MONTANTTOTAL    NUMBER(12,3)    DEFAULT 0,
    IDFOURNISSEUR   NUMBER(8)       CONSTRAINT NN_CMD_FOURNISSEUR NOT NULL,
    IDEMPLOYE       NUMBER(8)       CONSTRAINT NN_CMD_EMPLOYE NOT NULL,
    CONSTRAINT CK_CMD_MONTANT   CHECK (MONTANTTOTAL >= 0),
    CONSTRAINT CK_CMD_LIVRAISON CHECK (DATELIVRAISON IS NULL OR DATELIVRAISON >= DATECOMMANDE),
    CONSTRAINT FK_CMD_FOURNISSEUR FOREIGN KEY (IDFOURNISSEUR) REFERENCES FOURNISSEUR (IDFOURNISSEUR),
    CONSTRAINT FK_CMD_EMPLOYE     FOREIGN KEY (IDEMPLOYE)     REFERENCES EMPLOYE (IDEMPLOYE)
);

-- ---------------------------------------------------------------------
--  LIGNE_CMD (#IdCommande, #IdProduit, Quantite, PrixUnitaire)
--  Association « Contenir » : Commande (1,n) — Stock (0,n)
--  Clé primaire composée des deux clés étrangères
-- ---------------------------------------------------------------------
CREATE TABLE LIGNE_CMD (
    IDCOMMANDE      NUMBER(8),
    IDPRODUIT       NUMBER(8),
    QUANTITE        NUMBER(8)       CONSTRAINT NN_LIGNE_QUANTITE NOT NULL,
    PRIXUNITAIRE    NUMBER(10,3)    CONSTRAINT NN_LIGNE_PRIX NOT NULL,
    CONSTRAINT PK_LIGNE_CMD PRIMARY KEY (IDCOMMANDE, IDPRODUIT),
    CONSTRAINT CK_LIGNE_QUANTITE CHECK (QUANTITE > 0),
    CONSTRAINT CK_LIGNE_PRIX     CHECK (PRIXUNITAIRE >= 0),
    CONSTRAINT FK_LIGNE_COMMANDE FOREIGN KEY (IDCOMMANDE) REFERENCES COMMANDE (IDCOMMANDE) ON DELETE CASCADE,
    CONSTRAINT FK_LIGNE_STOCK    FOREIGN KEY (IDPRODUIT)  REFERENCES STOCK (IDPRODUIT)
);

-- ---------------------------------------------------------------------
--  CONSOMMATION (IdConso, #IdProduit, Quantite, DateConso)
--  Table ajoutée pour le module Stock (à faire valider par le tuteur)
-- ---------------------------------------------------------------------
CREATE TABLE CONSOMMATION (
    IDCONSO         NUMBER(10)      CONSTRAINT PK_CONSOMMATION PRIMARY KEY,
    IDPRODUIT       NUMBER(8)       CONSTRAINT NN_CONSO_PRODUIT NOT NULL,
    QUANTITE        NUMBER(8)       CONSTRAINT NN_CONSO_QUANTITE NOT NULL,
    DATECONSO       DATE            DEFAULT SYSDATE CONSTRAINT NN_CONSO_DATE NOT NULL,
    CONSTRAINT CK_CONSO_QUANTITE CHECK (QUANTITE > 0),
    CONSTRAINT FK_CONSO_STOCK FOREIGN KEY (IDPRODUIT) REFERENCES STOCK (IDPRODUIT) ON DELETE CASCADE
);

CREATE SEQUENCE SEQ_CONSOMMATION START WITH 1 INCREMENT BY 1 NOCACHE;

CREATE OR REPLACE TRIGGER TRG_CONSOMMATION_ID
BEFORE INSERT ON CONSOMMATION
FOR EACH ROW
WHEN (NEW.IDCONSO IS NULL)
BEGIN
    SELECT SEQ_CONSOMMATION.NEXTVAL INTO :NEW.IDCONSO FROM DUAL;
END;
/

-- Vérification : les 8 tables doivent apparaître
SELECT TABLE_NAME FROM USER_TABLES ORDER BY TABLE_NAME;

-- Vérification des clés étrangères
SELECT TABLE_NAME, CONSTRAINT_NAME FROM USER_CONSTRAINTS
WHERE CONSTRAINT_TYPE = 'R' ORDER BY TABLE_NAME;
