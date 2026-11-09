#include "langue.h"

#include <QHash>
#include <QSettings>

namespace {

struct Entree {
    const char *cle;
    const char *fr;
    const char *en;
    const char *ar;
};

const Entree TABLE[] = {
    // ---------------------------------------------------------------- menu latéral
    {"menu_accueil",      "Accueil",       "Home",         "الرئيسية"},
    {"menu_animaux",      "Animaux",       "Animals",      "الحيوانات"},
    {"menu_rdv",          "Rendez-vous",   "Appointments", "المواعيد"},
    {"menu_stock",        "Stock",         "Stock",        "المخزون"},
    {"menu_commandes",    "Commandes",     "Orders",       "الطلبات"},
    {"menu_fournisseurs", "Fournisseurs",  "Suppliers",    "الموردون"},
    {"menu_employes",     "Employés",      "Employees",    "الموظفون"},
    {"menu_parametres",   "Paramètres",    "Settings",     "الإعدادات"},

    // ---------------------------------------------------------------- général
    {"titre_erreur",  "Erreur",        "Error",        "خطأ"},
    {"titre_succes",  "Succès",        "Success",      "نجاح"},
    {"titre_confirm", "Confirmation",  "Confirmation", "تأكيد"},
    {"btn_oui",       "Oui",           "Yes",          "نعم"},
    {"btn_annuler",   "Annuler",       "Cancel",       "إلغاء"},

    // ---------------------------------------------------------------- rôles
    {"role_admin",    "Administrateur",           "Administrator",     "مدير"},
    {"role_vet",      "Vétérinaire",              "Veterinarian",      "طبيب بيطري"},
    {"role_stock",    "Gestionnaire de stock",    "Stock manager",     "مسؤول المخزون"},
    {"role_fourn",    "Responsable fournisseurs", "Suppliers manager", "مسؤول الموردين"},
    {"role_accueil",  "Réceptionniste",           "Receptionist",      "موظف الاستقبال"},

    // ---------------------------------------------------------------- connexion
    {"login_tagline", "Des animaux en bonne santé, des maîtres plus heureux !",
                      "Healthy pets, happier owners!",
                      "حيوانات بصحة جيدة، وأصحاب أكثر سعادة!"},
    {"login_points",  "🐾 Animaux   ·   📦 Stock   ·   🚚 Fournisseurs",
                      "🐾 Animals   ·   📦 Stock   ·   🚚 Suppliers",
                      "🐾 الحيوانات   ·   📦 المخزون   ·   🚚 الموردون"},
    {"login_titre",   "Connexion",                     "Sign in",                   "تسجيل الدخول"},
    {"login_sous",    "Bienvenue sur Pet Manager",     "Welcome to Pet Manager",    "مرحبًا بك في Pet Manager"},
    {"login_user",    "Utilisateur",                   "Username",                  "اسم المستخدم"},
    {"login_user_ph", "Votre nom d'utilisateur",       "Your username",             "اسم المستخدم الخاص بك"},
    {"login_pass",    "Mot de passe",                  "Password",                  "كلمة المرور"},
    {"login_pass_ph", "Votre mot de passe",            "Your password",             "كلمة المرور الخاصة بك"},
    {"login_oublie",  "Mot de passe oublié ?",         "Forgot password?",          "هل نسيت كلمة المرور؟"},
    {"login_bouton",  "Se connecter",                  "Sign in",                   "دخول"},
    {"login_profils", "Ou choisissez votre profil :",  "Or pick your profile:",     "أو اختر ملفك الشخصي:"},
    {"login_erreur",  "Utilisateur ou mot de passe incorrect.",
                      "Incorrect username or password.",
                      "اسم المستخدم أو كلمة المرور غير صحيحة."},
    {"login_vide",    "Veuillez saisir l'utilisateur et le mot de passe.",
                      "Please enter your username and password.",
                      "يرجى إدخال اسم المستخدم وكلمة المرور."},
    {"login_restant", "Il reste %1 essai(s).",         "%1 attempt(s) left.",       "تبقّى %1 محاولة."},
    {"login_bloque",  "Trop d'essais. Réessayez dans %1 s.",
                      "Too many attempts. Try again in %1 s.",
                      "محاولات كثيرة. أعد المحاولة بعد %1 ث."},
    {"login_voir",    "Afficher / masquer",            "Show / hide",               "إظهار / إخفاء"},

    // ---------------------------------------------------------------- mot de passe oublié / changé
    {"oubli_titre",   "Mot de passe oublié",           "Forgot password",           "نسيت كلمة المرور"},
    {"oubli_sous",    "Entrez l'adresse e-mail de votre compte : nous vous enverrons un code de vérification.",
                      "Enter your account's e-mail address: we will send you a verification code.",
                      "أدخل البريد الإلكتروني لحسابك: سنرسل إليك رمز تحقق."},
    {"oubli_email",   "Adresse e-mail",                "E-mail address",            "البريد الإلكتروني"},
    {"oubli_email_ph","exemple@gmail.com",             "example@gmail.com",         "example@gmail.com"},
    {"oubli_envoyer", "Envoyer le code",               "Send code",                 "إرسال الرمز"},
    {"oubli_envoi_cours", "Envoi en cours…",           "Sending…",                  "جارٍ الإرسال…"},
    {"oubli_err_email","Veuillez saisir une adresse e-mail valide.",
                       "Please enter a valid e-mail address.",
                       "يرجى إدخال بريد إلكتروني صالح."},
    {"oubli_email_inconnu","Aucun compte n'est associé à cette adresse e-mail.",
                           "No account is linked to this e-mail address.",
                           "لا يوجد حساب مرتبط بهذا البريد الإلكتروني."},
    {"oubli_err_envoi","Impossible d'envoyer l'e-mail.\n%1",
                       "Unable to send the e-mail.\n%1",
                       "تعذّر إرسال البريد الإلكتروني.\n%1"},
    {"oubli_code_envoye_mail", "📨 Un code à 6 chiffres a été envoyé à %1 (valable 10 minutes).",
                               "📨 A 6-digit code was sent to %1 (valid for 10 minutes).",
                               "📨 تم إرسال رمز من 6 أرقام إلى %1 (صالح لمدة 10 دقائق)."},
    {"oubli_code_envoye", "📨 Envoi d'e-mail non configuré (mode démo). Votre code : %1",
                          "📨 E-mail sending not configured (demo mode). Your code: %1",
                          "📨 إرسال البريد غير مُعدّ (وضع تجريبي). رمزك: %1"},
    {"oubli_etape_code", "Saisissez le code reçu par e-mail.",
                         "Enter the code you received by e-mail.",
                         "أدخل الرمز الذي وصلك عبر البريد الإلكتروني."},
    {"oubli_verifier","Vérifier le code",              "Verify code",               "تحقق من الرمز"},
    {"oubli_renvoyer","Renvoyer le code",              "Resend code",               "إعادة إرسال الرمز"},
    {"oubli_retour",  "Modifier l'e-mail",             "Change e-mail",             "تغيير البريد"},
    {"oubli_code_expire","Le code a expiré. Demandez un nouveau code.",
                         "The code has expired. Please request a new one.",
                         "انتهت صلاحية الرمز. اطلب رمزًا جديدًا."},
    {"oubli_trop_essais","Trop de codes incorrects. Demandez un nouveau code.",
                         "Too many wrong codes. Please request a new one.",
                         "رموز خاطئة كثيرة. اطلب رمزًا جديدًا."},
    {"oubli_etape_mdp","✔ Code vérifié. Choisissez votre nouveau mot de passe.",
                       "✔ Code verified. Choose your new password.",
                       "✔ تم التحقق من الرمز. اختر كلمة مرور جديدة."},
    {"oubli_changer", "Changer le mot de passe",       "Change password",           "تغيير كلمة المرور"},
    {"mail_sujet",    "Pet Manager - Code de réinitialisation",
                      "Pet Manager - Password reset code",
                      "Pet Manager - رمز إعادة تعيين كلمة المرور"},
    {"mail_corps",    "Bonjour,\n\nVotre code de vérification Pet Manager est : %1\n\nCe code est valable 10 minutes.\nSi vous n'avez pas demandé de réinitialisation, ignorez cet e-mail.\n\nL'équipe Pet Manager",
                      "Hello,\n\nYour Pet Manager verification code is: %1\n\nThis code is valid for 10 minutes.\nIf you did not request a reset, please ignore this e-mail.\n\nThe Pet Manager team",
                      "مرحبًا،\n\nرمز التحقق الخاص بك في Pet Manager هو: %1\n\nهذا الرمز صالح لمدة 10 دقائق.\nإذا لم تطلب إعادة التعيين، تجاهل هذه الرسالة.\n\nفريق Pet Manager"},
    {"oubli_code",    "Code de vérification",          "Verification code",         "رمز التحقق"},
    {"oubli_nouveau", "Nouveau mot de passe",          "New password",              "كلمة المرور الجديدة"},
    {"oubli_confirmer", "Confirmer le mot de passe",   "Confirm password",          "تأكيد كلمة المرور"},
    {"oubli_valider", "Réinitialiser",                 "Reset",                     "إعادة التعيين"},
    {"oubli_pas_code","Envoyez d'abord le code de vérification.",
                      "Please send the verification code first.",
                      "أرسل رمز التحقق أولًا."},
    {"oubli_err_code","Code de vérification incorrect.",
                      "Incorrect verification code.",
                      "رمز التحقق غير صحيح."},
    {"oubli_err_diff","Les deux mots de passe ne sont pas identiques.",
                      "The two passwords do not match.",
                      "كلمتا المرور غير متطابقتين."},
    {"oubli_err_court","Le mot de passe doit contenir au moins 4 caractères.",
                       "The password must contain at least 4 characters.",
                       "يجب أن تحتوي كلمة المرور على 4 أحرف على الأقل."},
    {"oubli_ok",      "Mot de passe modifié. Vous pouvez maintenant vous connecter.",
                      "Password changed. You can now sign in.",
                      "تم تغيير كلمة المرور. يمكنك الآن تسجيل الدخول."},
    {"mdp_titre",     "Changer mon mot de passe",      "Change my password",        "تغيير كلمة المرور"},
    {"mdp_actuel",    "Mot de passe actuel",           "Current password",          "كلمة المرور الحالية"},
    {"mdp_err_actuel","Le mot de passe actuel est incorrect.",
                      "The current password is incorrect.",
                      "كلمة المرور الحالية غير صحيحة."},
    {"mdp_ok",        "Votre mot de passe a été modifié.",
                      "Your password has been changed.",
                      "تم تغيير كلمة المرور الخاصة بك."},

    // ---------------------------------------------------------------- accueil
    {"acc_bonjour",   "Bonjour",       "Hello",         "مرحبًا"},
    {"acc_bonsoir",   "Bonsoir",       "Good evening",  "مساء الخير"},
    {"deconnexion",   "Déconnexion",   "Log out",       "تسجيل الخروج"},
    {"kpi_animaux",   "Animaux",              "Animals",           "الحيوانات"},
    {"kpi_produits",  "Produits en stock",    "Products in stock", "المنتجات في المخزون"},
    {"kpi_alertes",   "Alertes stock",        "Stock alerts",      "تنبيهات المخزون"},
    {"kpi_fournisseurs","Fournisseurs actifs","Active suppliers",  "الموردون النشطون"},
    {"det_surveillance","%1 sous surveillance","%1 under observation","%1 تحت المراقبة"},
    {"det_commandes", "%1 commande(s) en attente","%1 pending order(s)","%1 طلب قيد الانتظار"},
    {"det_alertes",   "%1 rupture · %2 stock bas · %3 expiré",
                      "%1 out of stock · %2 low · %3 expired",
                      "%1 نفاد · %2 منخفض · %3 منتهي"},
    {"det_sur",       "sur %1 fournisseur(s)", "out of %1 supplier(s)", "من أصل %1 مورّد"},
    {"acces_titre",   "Accès rapide",          "Quick access",          "وصول سريع"},
    {"surveiller_titre","À surveiller",        "To watch",              "للمتابعة"},
    {"resume_titre",  "Résumé",                "Summary",               "ملخص"},
    {"rien",          "Rien à signaler, tout va bien.",
                      "Nothing to report, all is well.",
                      "لا شيء يستدعي الانتباه، كل شيء على ما يرام."},
    {"autres",        "… et %1 autre(s)",      "… and %1 more",         "… و%1 آخر"},
    {"res_animaux",   "%1 chien(s) · %2 chat(s) · %3 lapin(s)",
                      "%1 dog(s) · %2 cat(s) · %3 rabbit(s)",
                      "%1 كلب · %2 قط · %3 أرنب"},
    {"res_stock",     "%1 produit(s) · %2 commande(s) en attente",
                      "%1 product(s) · %2 pending order(s)",
                      "%1 منتج · %2 طلب قيد الانتظار"},
    {"res_fourn",     "%1 au total · %2 actif(s)",
                      "%1 in total · %2 active",
                      "%1 في المجموع · %2 نشط"},
    {"res_employes",  "%1 au total · %2 actif(s)",
                      "%1 in total · %2 active",
                      "%1 في المجموع · %2 نشط"},
    {"al_rupture",    "rupture de stock",      "out of stock",          "نفاد المخزون"},
    {"al_bas",        "stock bas",             "low stock",             "مخزون منخفض"},
    {"al_expire",     "produit expiré",        "expired product",       "منتج منتهي الصلاحية"},
    {"astuce_titre",  "Astuce du jour",        "Tip of the day",        "نصيحة اليوم"},
    {"tip0", "Pensez à vérifier les dates d'expiration des médicaments chaque semaine.",
             "Remember to check medicine expiry dates every week.",
             "تذكّر فحص تواريخ انتهاء صلاحية الأدوية كل أسبوع."},
    {"tip1", "Pesez régulièrement les animaux : une variation de poids est un signal d'alerte précoce.",
             "Weigh animals regularly: a change in weight is an early warning sign.",
             "قم بوزن الحيوانات بانتظام: تغيّر الوزن علامة إنذار مبكرة."},
    {"tip2", "Commandez avant la rupture : le module Stock vous suggère les quantités à réapprovisionner.",
             "Order before running out: the Stock module suggests quantities to restock.",
             "اطلب قبل نفاد المخزون: تقترح وحدة المخزون الكميات المناسبة."},
    {"tip3", "Comparez vos fournisseurs : la fiabilité compte autant que le prix.",
             "Compare your suppliers: reliability matters as much as price.",
             "قارن بين مورّديك: الموثوقية لا تقل أهمية عن السعر."},
    {"tip4", "Gardez les coordonnées des propriétaires à jour pour les joindre rapidement.",
             "Keep owners' contact details up to date so you can reach them quickly.",
             "حافظ على تحديث بيانات الاتصال بأصحاب الحيوانات للوصول إليهم بسرعة."},
    {"tip5", "Pensez à changer votre mot de passe dans les Paramètres.",
             "Remember to change your password in Settings.",
             "تذكّر تغيير كلمة المرور من الإعدادات."},

    // ---------------------------------------------------------------- paramètres
    {"par_sous",      "Personnalisez l'application Pet Manager",
                      "Customize the Pet Manager application",
                      "خصّص تطبيق Pet Manager"},
    {"carte_compte",  "Compte",        "Account",       "الحساب"},
    {"compte_connecte","Connecté en tant que","Signed in as","مسجّل الدخول باسم"},
    {"changer_user",  "Changer d'utilisateur","Switch user","تغيير المستخدم"},
    {"confirm_logout","Voulez-vous vraiment changer d'utilisateur ?",
                      "Do you really want to switch user?",
                      "هل تريد فعلًا تغيير المستخدم؟"},
    {"carte_langue",  "Langue",        "Language",      "اللغة"},
    {"langue_label",  "Langue de l'application","Application language","لغة التطبيق"},
    {"langue_note",   "S'applique au menu, à la connexion, à l'accueil et aux paramètres.",
                      "Applies to the menu, sign-in, home and settings.",
                      "تُطبَّق على القائمة وتسجيل الدخول والرئيسية والإعدادات."},
    {"carte_general", "Général",       "General",       "عام"},
    {"nom_centre",    "Nom du centre", "Center name",   "اسم المركز"},
    {"nom_centre_ph", "Ex : Clinique vétérinaire de Tunis",
                      "e.g. Tunis Veterinary Clinic",
                      "مثال: العيادة البيطرية بتونس"},
    {"carte_demarrage","Démarrage",    "Startup",       "البدء"},
    {"page_demarrage","Page affichée après la connexion",
                      "Page shown after sign-in",
                      "الصفحة المعروضة بعد تسجيل الدخول"},
    {"carte_alertes", "Alertes",       "Alerts",        "التنبيهات"},
    {"alerte_sante",  "Afficher l'alerte santé à la première ouverture de la page Animaux",
                      "Show the health alert when the Animals page is first opened",
                      "عرض تنبيه الصحة عند أول فتح لصفحة الحيوانات"},
    {"carte_donnees", "Données",       "Data",          "البيانات"},
    {"base_label",    "Base de données (fournisseurs)","Database (suppliers)","قاعدة البيانات (الموردون)"},
    {"ouvrir_dossier","Ouvrir le dossier","Open folder","فتح المجلد"},
    {"carte_apropos", "À propos",      "About",         "حول"},
    {"apropos_texte", "<b>Pet Manager</b> — gestion d'un centre vétérinaire<br>Modules : Animaux · Stock · Fournisseurs<br>Version de Qt : %1",
                      "<b>Pet Manager</b> — veterinary center management<br>Modules: Animals · Stock · Suppliers<br>Qt version: %1",
                      "<b>Pet Manager</b> — إدارة مركز بيطري<br>الوحدات: الحيوانات · المخزون · الموردون<br>إصدار Qt: %1"},
    {"reinit",        "Réinitialiser", "Reset",         "إعادة الضبط"},
    {"enreg",         "Enregistrer",   "Save",          "حفظ"},
    {"ok_enreg",      "Vos paramètres ont été enregistrés.",
                      "Your settings have been saved.",
                      "تم حفظ إعداداتك."},
    {"confirm_reinit","Rétablir tous les paramètres par défaut ?",
                      "Restore all settings to their defaults?",
                      "استعادة جميع الإعدادات الافتراضية؟"},
};

const QHash<QString, const Entree *> &index()
{
    static QHash<QString, const Entree *> h;
    if (h.isEmpty()) {
        for (const Entree &e : TABLE)
            h.insert(QString::fromLatin1(e.cle), &e);
    }
    return h;
}

} // namespace

Langue::Langue()
{
    QSettings s("PetManager", "PetManager");
    const QString id = s.value("general/langue", "fr").toString();
    m_code = (id == "en") ? Anglais : (id == "ar") ? Arabe : Francais;
}

Langue &Langue::instance()
{
    static Langue l;
    return l;
}

Langue::Code Langue::courante()
{
    return instance().m_code;
}

QString Langue::t(const char *cle)
{
    const auto it = index().constFind(QString::fromLatin1(cle));
    if (it == index().constEnd())
        return QString::fromLatin1(cle);
    const Entree *e = it.value();
    const char *texte = e->fr;
    if (courante() == Anglais)    texte = e->en;
    else if (courante() == Arabe) texte = e->ar;
    return QString::fromUtf8(texte);
}

void Langue::definir(Code code)
{
    Langue &l = instance();
    if (l.m_code == code)
        return;
    l.m_code = code;

    QSettings s("PetManager", "PetManager");
    s.setValue("general/langue", code == Anglais ? "en" : code == Arabe ? "ar" : "fr");
    s.sync();

    emit l.langueChangee();
}

QString Langue::nomLangue(Code code)
{
    switch (code) {
    case Anglais: return "English";
    case Arabe:   return QString::fromUtf8("العربية");
    default:      return QString::fromUtf8("Français");
    }
}

Qt::LayoutDirection Langue::direction()
{
    return courante() == Arabe ? Qt::RightToLeft : Qt::LeftToRight;
}

QLocale Langue::locale()
{
    switch (courante()) {
    case Anglais: return QLocale(QLocale::English);
    case Arabe:   return QLocale(QLocale::Arabic);
    default:      return QLocale(QLocale::French);
    }
}
