package net.minecraft.util.text;

import com.google.common.base.Function;
import com.google.common.collect.Iterators;
import com.google.common.collect.Lists;
import java.util.Iterator;
import java.util.List;

public abstract class TextComponentBase implements ITextComponent
{
    protected List<ITextComponent> siblings = Lists.<ITextComponent>newArrayList();
    private Style style;

    /**
     * Appends the given component to the end of this one.
     */
    public ITextComponent appendSibling(ITextComponent component)
    {
        component.getChatStyle().setParentStyle(this.getChatStyle());
        this.siblings.add(component);
        return this;
    }

    public List<ITextComponent> getSiblings()
    {
        return this.siblings;
    }

    /**
     * Appends the given text to the end of this component.
     */
    public ITextComponent appendText(String text)
    {
        return this.appendSibling(new TextComponentString(text));
    }

    public ITextComponent setChatStyle(Style style)
    {
        this.style = style;

        for (ITextComponent itextcomponent : this.siblings)
        {
            itextcomponent.getChatStyle().setParentStyle(this.getChatStyle());
        }

        return this;
    }

    public Style getChatStyle()
    {
        if (this.style == null)
        {
            this.style = new Style();

            for (ITextComponent itextcomponent : this.siblings)
            {
                itextcomponent.getChatStyle().setParentStyle(this.style);
            }
        }

        return this.style;
    }

    public Iterator<ITextComponent> iterator()
    {
        return Iterators.<ITextComponent>concat(Iterators.<ITextComponent>forArray(new TextComponentBase[] {this}), createDeepCopyIterator(this.siblings));
    }

    /**
     * Get the text of this component, <em>and all child components</em>, with all special formatting codes removed.
     */
    public final String getUnformattedText()
    {
        StringBuilder stringbuilder = new StringBuilder();

        for (ITextComponent itextcomponent : this)
        {
            stringbuilder.append(itextcomponent.getUnformattedTextForChat());
        }

        return stringbuilder.toString();
    }

    /**
     * Gets the text of this component, with formatting codes added for rendering.
     */
    public final String getFormattedText()
    {
        StringBuilder stringbuilder = new StringBuilder();

        for (ITextComponent itextcomponent : this)
        {
            stringbuilder.append(itextcomponent.getChatStyle().getFormattingCode());
            stringbuilder.append(itextcomponent.getUnformattedTextForChat());
            stringbuilder.append((Object)TextFormatting.RESET);
        }

        return stringbuilder.toString();
    }

    public static Iterator<ITextComponent> createDeepCopyIterator(Iterable<ITextComponent> components)
    {
        Iterator<ITextComponent> iterator = Iterators.concat(Iterators.transform(components.iterator(), new Function<ITextComponent, Iterator<ITextComponent>>()
        {
            public Iterator<ITextComponent> apply(ITextComponent p_apply_1_)
            {
                return p_apply_1_.iterator();
            }
        }));
        iterator = Iterators.transform(iterator, new Function<ITextComponent, ITextComponent>()
        {
            public ITextComponent apply(ITextComponent p_apply_1_)
            {
                ITextComponent itextcomponent = p_apply_1_.createCopy();
                itextcomponent.setChatStyle(itextcomponent.getChatStyle().createDeepCopy());
                return itextcomponent;
            }
        });
        return iterator;
    }

    public boolean equals(Object p_equals_1_)
    {
        if (this == p_equals_1_)
        {
            return true;
        }
        else if (!(p_equals_1_ instanceof TextComponentBase))
        {
            return false;
        }
        else
        {
            TextComponentBase textcomponentbase = (TextComponentBase)p_equals_1_;
            return this.siblings.equals(textcomponentbase.siblings) && this.getChatStyle().equals(textcomponentbase.getChatStyle());
        }
    }

    public int hashCode()
    {
        return 31 * this.style.hashCode() + this.siblings.hashCode();
    }

    public String toString()
    {
        return "BaseComponent{style=" + this.style + ", siblings=" + this.siblings + '}';
    }
}
