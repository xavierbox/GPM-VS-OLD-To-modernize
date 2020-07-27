int littovel(){
  // Converts lithology, porosity, etc. to bulk density, Vp and Vs


  return 0;
}

/*
// Petrophysical transform using informations from GPM to generate elastic properties


// Output variables
frames {
        load layer_NOMAD50_Velocity.cell_50x50x20[20][50][50] {
        g       k ;
        g       mu ;
        g       rho ;
        g       vp ;
        g       vs ;
        g       vpvs ;
        }


        load layer_NOMAD50_Velocity.res_50x50x20[16] {
                S[85]       geometry;
        }


        load   LM_d_layer_NOMAD50_Velocity.mesh_51x51x21 [21][51][51] {
                c       vinfo;
                g       x;
                g       y;
                g       z;
         }
}


// Local variables
locals {
        g       k_cs5, mu_cs5, rho_cs5;
        g       k_ss15, mu_ss15, rho_ss15;
        g       k_ss35, mu_ss35, rho_ss35;
        g       k_ssh, mu_ssh, rho_ssh;
        g       k_sh, mu_sh, rho_sh;
        g       k_fluid, rho_fluid;
        g       porosity,v_clay;
        g       k_rock,mu_rock,rho_rock;
        g       k_tot_dry,mu_tot_dry,rho_tot_dry;
        g       kTot,muTot,rhoTot;
        g       ppressure,temperature,gas_saturation,oil_saturation,gas_gravity,GOR,oil_density,water_salinity; 
        g       rho_0,G,Rg,T,P,B_0,rho_prime,rho_g,rho_p,Vp,rho_live_oil,k_live_oil; 
        g       Ta,R,Pr,Tr,a,b,c,d,E,Z,m,f,gamma,K,rho,k_gas,rho_gas;
        g       S,rho_w,rho_B;
        g       omega [5][4];
        g       Vw,VB,rho_brine_water,k_brine_water,k_fluid_test,rho_fluid_test;
        g       cut_1,cut_2,cut_3,cut_4;
        g       vp,vs,vpvs;
}



// Establish new geometry
 @[:i:][:j:][:k:] {
LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.x [i][j][k] =     LM_d_layer_NOMAD_50km_1km_u.mesh_51x51x21.x [i][j][k];
LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.y [i][j][k] =     LM_d_layer_NOMAD_50km_1km_u.mesh_51x51x21.y [i][j][k];
LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.z [i][j][k] =     LM_d_layer_NOMAD_50km_1km_u.mesh_51x51x21.z [i][j][k];
LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.vinfo [i][j][k] = LM_d_layer_NOMAD_50km_1km_u.mesh_51x51x21.vinfo [i][j][k];
}


// New geometry file
@[:i:] {
layer_NOMAD50_Velocity.res_50x50x20.geometry [i] = layer_NOMAD_50km_1km_u.res_50x50x20.geometry [i];
}


// ------------------------------
// FLUIDS
// ------------------------------
//-- Input fluid properties
// Can be coming from somewhere else if same grid
//
ppressure           = 20*10^^6 ;  // ppressure in Pa   
temperature         = 273.15+80; // temperature  indegK   
// 1st model
//gas_saturation      = 0.05 ;     // ratio       
//oil_saturation      = 0.65 ;     // ratio
// 2nd model
//gas_saturation      = 0.90 ;     // ratio       
//oil_saturation      = 0.05 ;     // ratio
// 3rd model
gas_saturation      = 0.05 ;     // ratio       
oil_saturation      = 0.90 ;     // ratio
//
gas_gravity         = 0.6 ;      // ratio 
GOR                 = 100 ;      // ratio
oil_density         = 876 ;      // in kg/m^^3      
water_salinity      = 0.03 ;     // ppm/1E+6
//        
//-- live oil -- Batzle and wang relations
//
rho_0 = oil_density/1000 ;
G = gas_gravity ;
Rg = GOR ;
T = temperature - 273 ; 
P = ppressure*10^^(-6) ; 
//
B_0 = 0.972 + 0.00038*(2.4*Rg*sqrt(G/rho_0) + T + 1.78 )^^(1.175) ;
rho_prime = rho_0/(B_0*(1+0.001*Rg)) ;
rho_g = (rho_0 + 0.0012*G*Rg)/B_0 ;
rho_p = rho_g + (0.00277*P -1.71*(10^^(-7))*P^^(3))*(rho_g-1.15)^^2 + 3.49*(10^^(-4))*P ;
Vp = 2096*sqrt(rho_prime/(2.6-rho_prime)) - 3.7*T+4.64*P + 0.0115*(4.12*sqrt(1.08/rho_prime -1) -1 )*T*P ;
rho_live_oil = rho_p*1000 ;
k_live_oil = rho_live_oil*(Vp^^2) ; 
//
//-- gas -- Batzle and wang relations
//
P = ppressure/10^^6 ; 
Ta = temperature ; 
G = gas_gravity ;
//
R = 8.31441 ;
Pr = P/(4.892-0.4048*G) ;
Tr = Ta/(94.72+170.75*G) ;
a = 0.03+0.00527*(3.5-Tr)^^3 ;
b = 0.642*Tr-0.007*Tr^^4 -0.52 ;
c = 0.109*(3.85-Tr)^^2 ;
d = exp(-((Pr^^1.2)/Tr)*(0.45 + 8*(0.56-1/Tr)^^2)) ;
E = c*d ;
Z = a*Pr + b + E ;
m = 1.2*(-((Pr^^0.2)/Tr)*(0.45 + 8*(0.56-1/Tr)^^2)) ;
f = c*d*m + a ;
gamma = 0.85 + 5.6/(Pr +2) + 27.1/((Pr + 3.5)^^2) -8.7*exp(-0.65*(Pr+1)) ;
K = P*gamma/(1-Pr*f/Z) ;
rho = 28.8*G*P/(Z*R*Ta) ;
k_gas = K*10^^6 ;
rho_gas = rho*10^^3 ;
//
//-- brine water -- Batzle and wang relations
//
P = ppressure/10^^6 ;
T = temperature-273.15 ;
S = water_salinity ;
//
rho_w = 1+(10^^(-6))*(-80*T -3.3*T^^2 +0.00175*T^^3 + 489*P -2*T*P + 0.016*(T^^2)*P -1.3*(10^^(-5))*(T^^3)*P -0.333*P^^2 -0.002*T*P^^2) ;
rho_B = rho_w + S*(0.668 +0.44*S+(10^^(-6))*(300*P -2400*P*S + T*(80+3*T-3300*S-13*P+47*P*S))) ;
@[0:i:4][0:j:3] {omega[i][j]=0;} 
omega[0][0] = 1.40285*10^^3  ; 
omega[1][0] = 4.871 ; 
omega[2][0] = -4.783*10^^(-2) ; 
omega[3][0] = 1.487*10^^(-4) ; 
omega[4][0] = -2.197*10^^(-7) ; 
omega[0][1] = 1.524 ; 
omega[1][1] = -1.11*10^^(-2) ; 
omega[2][1] = 2.747*10^^(-4) ; 
omega[3][1] = -6.503*10^^(-7) ; 
omega[4][1] = 7.987*10^^(-10) ; 
omega[0][2] = 3.437*10^^(-3) ; 
omega[1][2] = 1.739*10^^(-4) ; 
omega[2][3] = -2.135*10^^(-6) ; 
omega[3][2] = -1.455*10^^(-8) ; 
omega[4][2] = 5.230*10^^(-11) ; 
omega[0][3] = -1.197*10^^(-5) ; 
omega[1][3] = -1.628*10^^(-6) ; 
omega[2][3] = 1.237*10^^(-8) ; 
omega[3][3] = 1.327*10^^(-10) ; 
omega[4][3] = -4.614*10^^(-13) ; 
Vw = 0 ; 
@[0:i:4][0:j:3] {Vw = Vw + omega[i][j]*(T^^i)*(P^^j);}     
VB = Vw + S*(1170-9.6*T+0.055*T^^2 -8.5*(10^^(-5))*T^^3 +2.6*P -0.0029*T*P -0.0476*P^^2) + (S^^(1.5))*(780-10*P+0.16*P^^2) - 1820*S^^2;
rho_brine_water = rho_B*1000 ;
k_brine_water = rho_brine_water*VB^^2 ;
//
//-- mixing fluids-- Wood's law
//
k_fluid = 1/(oil_saturation/k_live_oil + (1-oil_saturation-gas_saturation)/k_brine_water + gas_saturation/k_gas) ;
rho_fluid = oil_saturation*rho_live_oil + (1-oil_saturation-gas_saturation)*rho_brine_water + gas_saturation*rho_gas;
// ------------------------------
// End FLUIDS
// ------------------------------


// ------------------------------
// ROCK
// 5 standard rock type with moduli
//--------------------------------
// Clean sandstone (clay < 5%)
k_cs5 = 37.5E+9; 
mu_cs5 = 30.48E+9; 
rho_cs5 = 2.64E+3;
// Shaly Sandstone (5% < clay < 15%)
k_ss15 = 36.1E+9; 
mu_ss15 = 27.4E+9; 
rho_ss15 = 2.62E+3;
// Shaly Sanstone  (15% < clay < 35%)
k_ss35 = 32.3E+9; 
mu_ss35 = 21.3E+9; 
rho_ss35 = 2.59E+3;
// Sandy Shale (clay > 35%)
k_ssh = 25.5E+9; 
mu_ssh = 12.8E+9; 
rho_ssh = 2.5E+3;
// Shale (clay > 90 %)
k_sh = 20.0E+9; 
mu_sh = 7.6E+9; 
rho_sh = 2.4E+3;


cut_1 = 0.05;
cut_2 = 0.15;
cut_3 = 0.35;
cut_4 = 0.90;


 @[:i:][:j:][:k:] {
        porosity = layer_NOMAD_50km_1km_u.cell_50x50x20.poro [i][j][k];
        v_clay = 1 - layer_NOMAD_50km_1km_u.cell_50x50x20.netgross [i][j][k];
// Choosing non-porous elastic moduli
        if (v_clay <= cut_1 ) {
                k_rock = k_cs5;
                mu_rock = mu_cs5;
                rho_rock = rho_cs5;}
        else if ((v_clay <= cut_2) & (v_clay > cut_1)) {
                k_rock = k_ss15;
                mu_rock = mu_ss15;
                rho_rock = rho_ss15;}
        else if ((v_clay <= cut_3) && (v_clay > cut_2)) {
                k_rock = k_ss35;
                mu_rock = mu_ss35;
                rho_rock = rho_ss35;}
        else if ((v_clay <= cut_4) && (v_clay > cut_3)) {
                k_rock = k_ssh;
                mu_rock = mu_ssh;
                rho_rock = rho_ssh;}
        else {
                k_rock =  k_sh;
                mu_rock = mu_sh;
                rho_rock = rho_sh;}


// including porosity using Krief
        k_tot_dry = ((1 - porosity)^^(3/(1 - porosity))) * k_rock;
        mu_tot_dry = ((1 - porosity)^^(3/(1 - porosity))) * mu_rock;
        rho_tot_dry =(1 - porosity) * rho_rock;
// ------------------------------
// End ROCK
// ------------------------------


// ------------------------------
// ROCK + FLUID
// ------------------------------
rhoTot = rho_tot_dry + porosity * rho_fluid;
muTot = mu_tot_dry;
// classical gassman substitution
if (k_tot_dry != k_rock){
kTot = k_tot_dry + ((1 - k_tot_dry/k_rock)^^2)/(porosity/k_fluid + (1 - porosity)/k_rock - k_tot_dry/(k_rock^^2));
}
if (k_tot_dry == k_rock){
                kTot = k_tot_dry;}
// ------------------------------
// End ROCK + FLUID
// ------------------------------


// ------------------------------
// VELOCITIES
// ------------------------------
vp = sqrt((kTot + (4/3)*muTot)/rhoTot);
vs = sqrt(muTot/rhoTot);
vpvs = vp/vs;
// ------------------------------
// End VELOCITIES
// ------------------------------


layer_NOMAD50_Velocity.cell_50x50x20.k [i][j][k] = kTot;
layer_NOMAD50_Velocity.cell_50x50x20.mu [i][j][k] = muTot;
layer_NOMAD50_Velocity.cell_50x50x20.rho [i][j][k] = rhoTot;
layer_NOMAD50_Velocity.cell_50x50x20.vp [i][j][k] = vp;
layer_NOMAD50_Velocity.cell_50x50x20.vs [i][j][k] = vs;
layer_NOMAD50_Velocity.cell_50x50x20.vpvs [i][j][k] = vpvs;



}


update();


bwrite (LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.x);
bwrite (LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.y);
bwrite (LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.z);
bwrite (LM_d_layer_NOMAD50_Velocity.mesh_51x51x21.vinfo);
bwrite (layer_NOMAD50_Velocity.res_50x50x20.geometry);
bwrite (layer_NOMAD50_Velocity.cell_50x50x20.k);
bwrite (layer_NOMAD50_Velocity.cell_50x50x20.mu);
bwrite (layer_NOMAD50_Velocity.cell_50x50x20.rho);
bwrite (layer_NOMAD50_Velocity.cell_50x50x20.vp);
bwrite (layer_NOMAD50_Velocity.cell_50x50x20.vs);
bwrite (layer_NOMAD50_Velocity.cell_50x50x20.vpvs); 

  */