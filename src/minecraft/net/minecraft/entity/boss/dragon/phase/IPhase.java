package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragonPart;
import net.minecraft.entity.item.EntityEnderCrystal;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.DamageSource;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;

public interface IPhase
{
    boolean func_188654_a();

    void func_188657_b();

    void func_188659_c();

    void func_188655_a(EntityEnderCrystal p_188655_1_, BlockPos p_188655_2_, DamageSource p_188655_3_, EntityPlayer p_188655_4_);

    void func_188660_d();

    void func_188658_e();

    float func_188651_f();

    float func_188653_h();

    PhaseList <? extends IPhase > func_188652_i();

    Vec3d func_188650_g();

    float func_188656_a(EntityDragonPart p_188656_1_, DamageSource p_188656_2_, float p_188656_3_);
}
