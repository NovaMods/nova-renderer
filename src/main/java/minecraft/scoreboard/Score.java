package net.minecraft.scoreboard;

import java.util.Comparator;

public class Score
{
    public static final Comparator<Score> scoreComparator = new Comparator<Score>()
    {
        public int compare(Score p_compare_1_, Score p_compare_2_)
        {
            return p_compare_1_.getScorePoints() > p_compare_2_.getScorePoints() ? 1 : (p_compare_1_.getScorePoints() < p_compare_2_.getScorePoints() ? -1 : p_compare_2_.getPlayerName().compareToIgnoreCase(p_compare_1_.getPlayerName()));
        }
    };
    private final Scoreboard theScoreboard;
    private final ScoreObjective theScoreObjective;
    private final String scorePlayerName;
    private int scorePoints;
    private boolean locked;
    private boolean forceUpdate;

    public Score(Scoreboard theScoreboardIn, ScoreObjective theScoreObjectiveIn, String scorePlayerNameIn)
    {
        this.theScoreboard = theScoreboardIn;
        this.theScoreObjective = theScoreObjectiveIn;
        this.scorePlayerName = scorePlayerNameIn;
        this.forceUpdate = true;
    }

    public void increseScore(int amount)
    {
        if (this.theScoreObjective.getCriteria().isReadOnly())
        {
            throw new IllegalStateException("Cannot modify read-only score");
        }
        else
        {
            this.setScorePoints(this.getScorePoints() + amount);
        }
    }

    public void decreaseScore(int amount)
    {
        if (this.theScoreObjective.getCriteria().isReadOnly())
        {
            throw new IllegalStateException("Cannot modify read-only score");
        }
        else
        {
            this.setScorePoints(this.getScorePoints() - amount);
        }
    }

    public void func_96648_a()
    {
        if (this.theScoreObjective.getCriteria().isReadOnly())
        {
            throw new IllegalStateException("Cannot modify read-only score");
        }
        else
        {
            this.increseScore(1);
        }
    }

    public int getScorePoints()
    {
        return this.scorePoints;
    }

    public void setScorePoints(int points)
    {
        int i = this.scorePoints;
        this.scorePoints = points;

        if (i != points || this.forceUpdate)
        {
            this.forceUpdate = false;
            this.getScoreScoreboard().func_96536_a(this);
        }
    }

    public ScoreObjective getObjective()
    {
        return this.theScoreObjective;
    }

    /**
     * Returns the name of the player this score belongs to
     */
    public String getPlayerName()
    {
        return this.scorePlayerName;
    }

    public Scoreboard getScoreScoreboard()
    {
        return this.theScoreboard;
    }

    public boolean isLocked()
    {
        return this.locked;
    }

    public void setLocked(boolean locked)
    {
        this.locked = locked;
    }
}
