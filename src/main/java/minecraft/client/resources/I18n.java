package net.minecraft.client.resources;

public class I18n
{
    private static Locale i18nLocale;

    static void setLocale(Locale i18nLocaleIn)
    {
        i18nLocale = i18nLocaleIn;
    }

    /**
     * format(a, b) is equivalent to String.format(translate(a), b). Args: translationKey, params...
     */
    public static String format(String translateKey, Object... parameters)
    {
        return i18nLocale.formatMessage(translateKey, parameters);
    }

    public static boolean func_188566_a(String p_188566_0_)
    {
        return i18nLocale.func_188568_a(p_188566_0_);
    }
}
