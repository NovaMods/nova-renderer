package net.minecraft.entity;

import java.util.UUID;

public interface IEntityOwnable
{
    UUID getOwnerId();

    Entity getOwner();
}
