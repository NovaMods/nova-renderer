package net.minecraft.command;

public interface ICommandListener
{
    /**
     * Send an informative message to the server operators
     */
    void notifyOperators(ICommandSender sender, ICommand command, int flags, String msgFormat, Object... msgParams);
}
