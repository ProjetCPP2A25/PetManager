-- =====================================================================
--  Module Commandes : adaptation de la table COMMANDE
--  1) Le formulaire enregistre une quantité : colonne QUANTITE ajoutée.
--  2) Le formulaire ne demande pas encore l'employé (module Employés pas
--     encore intégré) : IDEMPLOYE doit accepter NULL.
--  À exécuter UNE FOIS, après base_commune_petmanager.sql.
-- =====================================================================
ALTER TABLE COMMANDE ADD (QUANTITE NUMBER(8) CONSTRAINT CK_CMD_QUANTITE CHECK (QUANTITE > 0));
ALTER TABLE COMMANDE MODIFY (IDEMPLOYE NULL);
