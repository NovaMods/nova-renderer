package net.minecraft.client.audio;

import java.util.Random;
import net.minecraft.client.Minecraft;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.ITickable;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.MathHelper;

public class MusicTicker implements ITickable
{
    private final Random rand = new Random();
    private final Minecraft mc;
    private ISound currentMusic;
    private int timeUntilNextMusic = 100;

    public MusicTicker(Minecraft mcIn)
    {
        this.mc = mcIn;
    }

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        MusicTicker.MusicType musicticker$musictype = this.mc.getAmbientMusicType();

        if (this.currentMusic != null)
        {
            if (!musicticker$musictype.func_188768_a().func_187503_a().equals(this.currentMusic.getSoundLocation()))
            {
                this.mc.getSoundHandler().stopSound(this.currentMusic);
                this.timeUntilNextMusic = MathHelper.getRandomIntegerInRange(this.rand, 0, musicticker$musictype.getMinDelay() / 2);
            }

            if (!this.mc.getSoundHandler().isSoundPlaying(this.currentMusic))
            {
                this.currentMusic = null;
                this.timeUntilNextMusic = Math.min(MathHelper.getRandomIntegerInRange(this.rand, musicticker$musictype.getMinDelay(), musicticker$musictype.getMaxDelay()), this.timeUntilNextMusic);
            }
        }

        this.timeUntilNextMusic = Math.min(this.timeUntilNextMusic, musicticker$musictype.getMaxDelay());

        if (this.currentMusic == null && this.timeUntilNextMusic-- <= 0)
        {
            this.func_181558_a(musicticker$musictype);
        }
    }

    public void func_181558_a(MusicTicker.MusicType p_181558_1_)
    {
        this.currentMusic = PositionedSoundRecord.func_184370_a(p_181558_1_.func_188768_a());
        this.mc.getSoundHandler().playSound(this.currentMusic);
        this.timeUntilNextMusic = Integer.MAX_VALUE;
    }

    public void func_181557_a()
    {
        if (this.currentMusic != null)
        {
            this.mc.getSoundHandler().stopSound(this.currentMusic);
            this.currentMusic = null;
            this.timeUntilNextMusic = 0;
        }
    }

    public static enum MusicType
    {
        MENU(SoundEvents.music_menu, 20, 600),
        GAME(SoundEvents.music_game, 12000, 24000),
        CREATIVE(SoundEvents.music_creative, 1200, 3600),
        CREDITS(SoundEvents.music_credits, Integer.MAX_VALUE, Integer.MAX_VALUE),
        NETHER(SoundEvents.music_nether, 1200, 3600),
        END_BOSS(SoundEvents.music_dragon, 0, 0),
        END(SoundEvents.music_end, 6000, 24000);

        private final SoundEvent musicLocation;
        private final int minDelay;
        private final int maxDelay;

        private MusicType(SoundEvent p_i47050_3_, int p_i47050_4_, int p_i47050_5_)
        {
            this.musicLocation = p_i47050_3_;
            this.minDelay = p_i47050_4_;
            this.maxDelay = p_i47050_5_;
        }

        public SoundEvent func_188768_a()
        {
            return this.musicLocation;
        }

        public int getMinDelay()
        {
            return this.minDelay;
        }

        public int getMaxDelay()
        {
            return this.maxDelay;
        }
    }
}
