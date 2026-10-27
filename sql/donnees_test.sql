-- NOTE ENCODAGE : les textes accentués sont écrits avec UNISTR('\00E9' = é ...)
-- pour que les accents soient corrects quel que soit l'encodage de SQL Developer.
-- =====================================================================
--  PET MANAGER - Données de test de la base commune
--  À exécuter APRÈS base_commune_petmanager.sql (bouton « Exécuter le script »)
--  Le script peut être relancé : il vide d'abord les tables.
-- =====================================================================

-- Vider les tables (ordre inverse des clés étrangères)
DELETE FROM CONSOMMATION;
DELETE FROM LIGNE_CMD;
DELETE FROM COMMANDE;
DELETE FROM RENDEZVOUS;
DELETE FROM STOCK;
DELETE FROM FOURNISSEUR;
DELETE FROM EMPLOYE;
DELETE FROM ANIMAL;

-- ---------------------------- FOURNISSEUR ----------------------------
INSERT INTO FOURNISSEUR VALUES (1, 'PharmaVet Tunisie', 'Karim Jaziri',  '71 234 567', 'contact@pharmavet.tn',  'Zone industrielle, Ben Arous', UNISTR('M\00E9dicament'),   'Actif', ADD_MONTHS(TRUNC(SYSDATE), -24));
INSERT INTO FOURNISSEUR VALUES (2, 'Royal Pet Food',    'Sonia Mansour', '71 456 789', 'ventes@royalpet.tn',    'Rue de Marseille, Tunis',      'Alimentation', 'Actif', ADD_MONTHS(TRUNC(SYSDATE), -18));
INSERT INTO FOURNISSEUR VALUES (3, UNISTR('Animalis \00C9quipement'),'Walid Gharbi', '73 321 654', 'info@animalis.tn',      'Avenue Habib Bourguiba, Sousse',UNISTR('\00C9quipement'),  'Actif', ADD_MONTHS(TRUNC(SYSDATE), -12));
INSERT INTO FOURNISSEUR VALUES (4, 'MedEquip Vet',      'Nadia Ferchichi','74 987 321','commandes@medequip.tn', 'Route de Tunis, Sfax',         UNISTR('\00C9quipement'),   'Actif', ADD_MONTHS(TRUNC(SYSDATE), -6));

-- ------------------------------ EMPLOYE ------------------------------
INSERT INTO EMPLOYE VALUES (1, 'Ben Ali',  'Sami',   'sami.benali@petmanager.tn',   '98 111 222', UNISTR('V\00E9t\00E9rinaire'),          2800, ADD_MONTHS(TRUNC(SYSDATE), -36), 'Actif');
INSERT INTO EMPLOYE VALUES (2, 'Matoussi', 'Chahed', 'chahed.matoussi@petmanager.tn','98 333 444', 'Responsable du stock', 1600, ADD_MONTHS(TRUNC(SYSDATE), -12), 'Actif');
INSERT INTO EMPLOYE VALUES (3, 'Trabelsi', 'Ines',   'ines.trabelsi@petmanager.tn',  '98 555 666', 'Assistante',           1200, ADD_MONTHS(TRUNC(SYSDATE), -8),  'Actif');

-- ------------------------------ ANIMAL -------------------------------
INSERT INTO ANIMAL VALUES (1, NULL, 'Rex',  'Chien', 'Berger allemand', UNISTR('M\00E2le'),    ADD_MONTHS(TRUNC(SYSDATE), -48), 32.5, UNISTR('Vaccins \00E0 jour'));
INSERT INTO ANIMAL VALUES (2, NULL, 'Mimi', 'Chat',  'Persan',          'Femelle', ADD_MONTHS(TRUNC(SYSDATE), -30), 4.2,  'Allergie alimentaire');
INSERT INTO ANIMAL VALUES (3, NULL, 'Max',  'Chien', 'Labrador',        UNISTR('M\00E2le'),    ADD_MONTHS(TRUNC(SYSDATE), -14), 24.0, 'Suivi de croissance');

-- ---------------------------- RENDEZVOUS -----------------------------
INSERT INTO RENDEZVOUS VALUES (1, TRUNC(SYSDATE) + 2, '09:30', UNISTR('Confirm\00E9'),   'Vaccination annuelle', 1, 1);
INSERT INTO RENDEZVOUS VALUES (2, TRUNC(SYSDATE) + 5, '14:00', 'En attente', UNISTR('Contr\00F4le dermatologique'), 2, 1);

-- ------------------------------- STOCK -------------------------------
INSERT INTO STOCK VALUES (1,  'Croquettes premium chien',    'Alimentation', 12, 5,  85,   ADD_MONTHS(TRUNC(SYSDATE), 11), UNISTR('R\00E9serve A - \00C9tag\00E8re 1'), 'Disponible');
INSERT INTO STOCK VALUES (2,  'Antibiotique (Amoxicilline)', UNISTR('M\00E9dicament'),    3, 5,  24.5, ADD_MONTHS(TRUNC(SYSDATE), 11), 'Pharmacie - Tiroir 2',  'Stock bas');
INSERT INTO STOCK VALUES (3,  'Vaccin CHPP',                 UNISTR('M\00E9dicament'),    0, 2,  45,   TRUNC(SYSDATE) + 120,           UNISTR('R\00E9frig\00E9rateur 1'),       'Rupture');
INSERT INTO STOCK VALUES (4,  UNISTR('Laisse r\00E9glable'),             UNISTR('\00C9quipement'),   18, 10, 18,   ADD_MONTHS(TRUNC(SYSDATE), 24), 'Boutique - Rayon 3',    'Disponible');
INSERT INTO STOCK VALUES (5,  UNISTR('Thermom\00E8tre v\00E9t\00E9rinaire'),     UNISTR('\00C9quipement'),    4, 5,  32,   ADD_MONTHS(TRUNC(SYSDATE), 24), 'Salle de soins',        'Stock bas');
INSERT INTO STOCK VALUES (6,  UNISTR('D\00E9sinfectant 500ml'),          UNISTR('M\00E9dicament'),    6, 3,  15,   TRUNC(SYSDATE) - 18,            'Pharmacie - Tiroir 4',  'Expire');
INSERT INTO STOCK VALUES (7,  UNISTR('D\00E9sinfectant 250ml'),          UNISTR('M\00E9dicament'),   13, 4,  9,    ADD_MONTHS(TRUNC(SYSDATE), 8),  'Pharmacie - Tiroir 4',  'Disponible');
INSERT INTO STOCK VALUES (8,  'Aliment humide chat',         'Alimentation', 20, 8,  6.5,  TRUNC(SYSDATE) + 20,            UNISTR('R\00E9serve A - \00C9tag\00E8re 2'), 'Disponible');
INSERT INTO STOCK VALUES (9,  'Aliment sec chat',            'Alimentation', 13, 5,  42,   ADD_MONTHS(TRUNC(SYSDATE), 9),  UNISTR('R\00E9serve A - \00C9tag\00E8re 2'), 'Disponible');
INSERT INTO STOCK VALUES (10, 'Antibiotique (Doxycycline)',  UNISTR('M\00E9dicament'),    9, 3,  28,   ADD_MONTHS(TRUNC(SYSDATE), 10), 'Pharmacie - Tiroir 2',  'Disponible');
INSERT INTO STOCK VALUES (11, 'Vaccin CHPPi-L',              UNISTR('M\00E9dicament'),    6, 2,  52,   ADD_MONTHS(TRUNC(SYSDATE), 6),  UNISTR('R\00E9frig\00E9rateur 1'),       'Disponible');
INSERT INTO STOCK VALUES (12, 'Croquettes standard chien',   'Alimentation', 25, 8,  60,   ADD_MONTHS(TRUNC(SYSDATE), 12), UNISTR('R\00E9serve A - \00C9tag\00E8re 1'), 'Disponible');
INSERT INTO STOCK VALUES (13, UNISTR('Vermifuge chat (comprim\00E9s)'),  UNISTR('M\00E9dicament'),    0, 4,  19,   ADD_MONTHS(TRUNC(SYSDATE), 14), 'Pharmacie - Tiroir 1',  'Rupture');
INSERT INTO STOCK VALUES (14, 'Pipette antiparasitaire chat',UNISTR('M\00E9dicament'),   10, 4,  22,   ADD_MONTHS(TRUNC(SYSDATE), 14), 'Pharmacie - Tiroir 1',  'Disponible');
INSERT INTO STOCK VALUES (15, 'Cage de transport',           UNISTR('\00C9quipement'),    6, 2,  120,  ADD_MONTHS(TRUNC(SYSDATE), 36), 'Boutique - Rayon 1',    'Disponible');

-- ----------------------------- COMMANDE ------------------------------
-- (IDCOMMANDE, DATECOMMANDE, DATELIVRAISON, STATUT, MONTANTTOTAL, #IDFOURNISSEUR, #IDEMPLOYE)
INSERT INTO COMMANDE VALUES (1, TRUNC(SYSDATE) - 40, TRUNC(SYSDATE) - 37, UNISTR('Livr\00E9e'), 0, 1, 2);
INSERT INTO COMMANDE VALUES (2, TRUNC(SYSDATE) - 35, TRUNC(SYSDATE) - 33, UNISTR('Livr\00E9e'), 0, 2, 2);
INSERT INTO COMMANDE VALUES (3, TRUNC(SYSDATE) - 30, TRUNC(SYSDATE) - 26, UNISTR('Livr\00E9e'), 0, 3, 2);
INSERT INTO COMMANDE VALUES (4, TRUNC(SYSDATE) - 25, TRUNC(SYSDATE) - 22, UNISTR('Livr\00E9e'), 0, 4, 2);
INSERT INTO COMMANDE VALUES (5, TRUNC(SYSDATE) - 20, TRUNC(SYSDATE) - 17, UNISTR('Livr\00E9e'), 0, 1, 2);

-- ----------------------------- LIGNE_CMD -----------------------------
-- (#IDCOMMANDE, #IDPRODUIT, QUANTITE, PRIXUNITAIRE)
INSERT INTO LIGNE_CMD VALUES (1, 2,  10, 24.5);
INSERT INTO LIGNE_CMD VALUES (1, 3,  4,  45);
INSERT INTO LIGNE_CMD VALUES (1, 6,  6,  15);
INSERT INTO LIGNE_CMD VALUES (1, 7,  15, 9);
INSERT INTO LIGNE_CMD VALUES (2, 1,  20, 85);
INSERT INTO LIGNE_CMD VALUES (2, 8,  40, 6.5);
INSERT INTO LIGNE_CMD VALUES (2, 9,  25, 42);
INSERT INTO LIGNE_CMD VALUES (3, 4,  20, 18);
INSERT INTO LIGNE_CMD VALUES (3, 12, 40, 60);
INSERT INTO LIGNE_CMD VALUES (3, 15, 6,  120);
INSERT INTO LIGNE_CMD VALUES (4, 5,  8,  32);
INSERT INTO LIGNE_CMD VALUES (5, 10, 12, 28);
INSERT INTO LIGNE_CMD VALUES (5, 11, 6,  52);
INSERT INTO LIGNE_CMD VALUES (5, 13, 8,  19);
INSERT INTO LIGNE_CMD VALUES (5, 14, 12, 22);

-- Montant total de chaque commande = somme de ses lignes
UPDATE COMMANDE c
SET MONTANTTOTAL = (SELECT SUM(l.QUANTITE * l.PRIXUNITAIRE) FROM LIGNE_CMD l
                    WHERE l.IDCOMMANDE = c.IDCOMMANDE);

-- --------------------------- CONSOMMATION ----------------------------
-- 30 derniers jours de sorties de stock (IDCONSO rempli par le trigger)
INSERT INTO CONSOMMATION (IDPRODUIT, QUANTITE, DATECONSO)
SELECT p.IDPRODUIT,
       MOD(p.IDPRODUIT * 7 + j.n * 3, 4) + 1,
       TRUNC(SYSDATE) - j.n
FROM STOCK p,
     (SELECT LEVEL - 1 AS n FROM DUAL CONNECT BY LEVEL <= 30) j
WHERE p.IDPRODUIT IN (1, 2, 7, 8, 9, 10, 12, 14)
  AND MOD(p.IDPRODUIT + j.n, 3) <> 0;

COMMIT;

-- ---------------------------- Vérification ---------------------------
SELECT 'FOURNISSEUR' AS TABLE_NAME, COUNT(*) AS NB_LIGNES FROM FOURNISSEUR UNION ALL
SELECT 'EMPLOYE',      COUNT(*) FROM EMPLOYE      UNION ALL
SELECT 'ANIMAL',       COUNT(*) FROM ANIMAL       UNION ALL
SELECT 'RENDEZVOUS',   COUNT(*) FROM RENDEZVOUS   UNION ALL
SELECT 'STOCK',        COUNT(*) FROM STOCK        UNION ALL
SELECT 'COMMANDE',     COUNT(*) FROM COMMANDE     UNION ALL
SELECT 'LIGNE_CMD',    COUNT(*) FROM LIGNE_CMD    UNION ALL
SELECT 'CONSOMMATION', COUNT(*) FROM CONSOMMATION;

-- Fournisseur de chaque produit, retrouvé par les commandes (utilisé par la fiche Excel)
SELECT s.IDPRODUIT, s.NOM, f.NOM AS FOURNISSEUR
FROM STOCK s
JOIN LIGNE_CMD l ON l.IDPRODUIT = s.IDPRODUIT
JOIN COMMANDE c ON c.IDCOMMANDE = l.IDCOMMANDE
JOIN FOURNISSEUR f ON f.IDFOURNISSEUR = c.IDFOURNISSEUR
ORDER BY s.IDPRODUIT;
