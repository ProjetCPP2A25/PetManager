-- NOTE ENCODAGE : les textes accentués sont écrits avec UNISTR('\00E9' = é ...)
-- pour que les accents soient corrects quel que soit l'encodage de SQL Developer.
-- =====================================================================
--  PET MANAGER - Données de test du MODULE STOCK (Chahed Matoussi)
--  Ne remplit que les tables du module : STOCK et CONSOMMATION.
--  Les autres tables (ANIMAL, EMPLOYE, FOURNISSEUR, COMMANDE, LIGNE_CMD...)
--  sont remplies par leurs responsables respectifs.
--  À exécuter APRÈS base_commune_petmanager.sql. Peut être relancé.
-- =====================================================================

-- Vider les tables du module (CONSOMMATION d'abord : clé étrangère vers STOCK)
DELETE FROM CONSOMMATION;
DELETE FROM STOCK;

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
SELECT 'STOCK' AS TABLE_NAME, COUNT(*) AS NB_LIGNES FROM STOCK UNION ALL
SELECT 'CONSOMMATION',        COUNT(*)              FROM CONSOMMATION;
