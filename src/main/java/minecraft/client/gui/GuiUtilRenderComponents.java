package net.minecraft.client.gui;

import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.client.Minecraft;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextFormatting;

public class GuiUtilRenderComponents
{
    public static String func_178909_a(String p_178909_0_, boolean p_178909_1_)
    {
        return !p_178909_1_ && !Minecraft.getMinecraft().gameSettings.chatColours ? TextFormatting.getTextWithoutFormattingCodes(p_178909_0_) : p_178909_0_;
    }

    public static List<ITextComponent> splitText(ITextComponent p_178908_0_, int p_178908_1_, FontRenderer p_178908_2_, boolean p_178908_3_, boolean p_178908_4_)
    {
        int i = 0;
        ITextComponent itextcomponent = new TextComponentString("");
        List<ITextComponent> list = Lists.<ITextComponent>newArrayList();
        List<ITextComponent> list1 = Lists.newArrayList(p_178908_0_);

        for (int j = 0; j < ((List)list1).size(); ++j)
        {
            ITextComponent itextcomponent1 = (ITextComponent)list1.get(j);
            String s = itextcomponent1.getUnformattedTextForChat();
            boolean flag = false;

            if (s.contains("\n"))
            {
                int k = s.indexOf(10);
                String s1 = s.substring(k + 1);
                s = s.substring(0, k + 1);
                TextComponentString textcomponentstring = new TextComponentString(s1);
                textcomponentstring.setChatStyle(itextcomponent1.getChatStyle().createShallowCopy());
                list1.add(j + 1, textcomponentstring);
                flag = true;
            }

            String s4 = func_178909_a(itextcomponent1.getChatStyle().getFormattingCode() + s, p_178908_4_);
            String s5 = s4.endsWith("\n") ? s4.substring(0, s4.length() - 1) : s4;
            int i1 = p_178908_2_.getStringWidth(s5);
            TextComponentString textcomponentstring1 = new TextComponentString(s5);
            textcomponentstring1.setChatStyle(itextcomponent1.getChatStyle().createShallowCopy());

            if (i + i1 > p_178908_1_)
            {
                String s2 = p_178908_2_.trimStringToWidth(s4, p_178908_1_ - i, false);
                String s3 = s2.length() < s4.length() ? s4.substring(s2.length()) : null;

                if (s3 != null && !s3.isEmpty())
                {
                    int l = s2.lastIndexOf(" ");

                    if (l >= 0 && p_178908_2_.getStringWidth(s4.substring(0, l)) > 0)
                    {
                        s2 = s4.substring(0, l);

                        if (p_178908_3_)
                        {
                            ++l;
                        }

                        s3 = s4.substring(l);
                    }
                    else if (i > 0 && !s4.contains(" "))
                    {
                        s2 = "";
                        s3 = s4;
                    }

                    TextComponentString textcomponentstring2 = new TextComponentString(s3);
                    textcomponentstring2.setChatStyle(itextcomponent1.getChatStyle().createShallowCopy());
                    list1.add(j + 1, textcomponentstring2);
                }

                i1 = p_178908_2_.getStringWidth(s2);
                textcomponentstring1 = new TextComponentString(s2);
                textcomponentstring1.setChatStyle(itextcomponent1.getChatStyle().createShallowCopy());
                flag = true;
            }

            if (i + i1 <= p_178908_1_)
            {
                i += i1;
                itextcomponent.appendSibling(textcomponentstring1);
            }
            else
            {
                flag = true;
            }

            if (flag)
            {
                list.add(itextcomponent);
                i = 0;
                itextcomponent = new TextComponentString("");
            }
        }

        list.add(itextcomponent);
        return list;
    }
}
