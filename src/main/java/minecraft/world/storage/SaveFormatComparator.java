package net.minecraft.world.storage;

import net.minecraft.util.StringUtils;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.WorldSettings;

public class SaveFormatComparator implements Comparable<SaveFormatComparator>
{
    /** the file name of this save */
    private final String fileName;

    /** the displayed name of this save file */
    private final String displayName;
    private final long lastTimePlayed;
    private final long sizeOnDisk;
    private final boolean requiresConversion;

    /** Instance of EnumGameType. */
    private final WorldSettings.GameType theEnumGameType;
    private final boolean hardcore;
    private final boolean cheatsEnabled;
    private final String versionName;
    private final int versionId;
    private final boolean versionSnapshot;

    public SaveFormatComparator(WorldInfo info, String p_i46646_2_, String p_i46646_3_, long p_i46646_4_, boolean p_i46646_6_)
    {
        this.fileName = p_i46646_2_;
        this.displayName = p_i46646_3_;
        this.lastTimePlayed = info.getLastTimePlayed();
        this.sizeOnDisk = p_i46646_4_;
        this.theEnumGameType = info.getGameType();
        this.requiresConversion = p_i46646_6_;
        this.hardcore = info.isHardcoreModeEnabled();
        this.cheatsEnabled = info.areCommandsAllowed();
        this.versionName = info.getVersionName();
        this.versionId = info.getVersionId();
        this.versionSnapshot = info.isVersionSnapshot();
    }

    /**
     * return the file name
     */
    public String getFileName()
    {
        return this.fileName;
    }

    /**
     * return the display name of the save
     */
    public String getDisplayName()
    {
        return this.displayName;
    }

    public long getSizeOnDisk()
    {
        return this.sizeOnDisk;
    }

    public boolean requiresConversion()
    {
        return this.requiresConversion;
    }

    public long getLastTimePlayed()
    {
        return this.lastTimePlayed;
    }

    public int compareTo(SaveFormatComparator p_compareTo_1_)
    {
        return this.lastTimePlayed < p_compareTo_1_.lastTimePlayed ? 1 : (this.lastTimePlayed > p_compareTo_1_.lastTimePlayed ? -1 : this.fileName.compareTo(p_compareTo_1_.fileName));
    }

    /**
     * Gets the EnumGameType.
     */
    public WorldSettings.GameType getEnumGameType()
    {
        return this.theEnumGameType;
    }

    public boolean isHardcoreModeEnabled()
    {
        return this.hardcore;
    }

    /**
     * @return {@code true} if cheats are enabled for this world
     */
    public boolean getCheatsEnabled()
    {
        return this.cheatsEnabled;
    }

    public String func_186357_i()
    {
        return StringUtils.isNullOrEmpty(this.versionName) ? I18n.translateToLocal("selectWorld.versionUnknown") : this.versionName;
    }

    public boolean func_186355_l()
    {
        return this.func_186356_m();
    }

    public boolean func_186356_m()
    {
        return this.versionId > 169;
    }
}
