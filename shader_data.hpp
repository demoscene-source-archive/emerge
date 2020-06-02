struct ShaderSymbol
{
    char character;
    unsigned short int frequency;
};
static const int shader_symbol_count=83;
extern const ShaderSymbol shader_symbols[shader_symbol_count];
extern const unsigned char particle_update_vertex_glsl[128];
static const unsigned long int particle_update_vertex_glsl_bitcount = 1021;
extern const unsigned char particle_update_fragment_glsl[777];
static const unsigned long int particle_update_fragment_glsl_bitcount = 6214;
extern const unsigned char particle_init_vertex_glsl[128];
static const unsigned long int particle_init_vertex_glsl_bitcount = 1021;
extern const unsigned char particle_init_fragment_glsl[991];
static const unsigned long int particle_init_fragment_glsl_bitcount = 7921;
extern const unsigned char particle_draw_vertex_glsl[1059];
static const unsigned long int particle_draw_vertex_glsl_bitcount = 8471;
extern const unsigned char particle_draw_fragment_glsl[81];
static const unsigned long int particle_draw_fragment_glsl_bitcount = 642;
extern const unsigned char particle_plot_vertex_glsl[332];
static const unsigned long int particle_plot_vertex_glsl_bitcount = 2653;
extern const unsigned char particle_plot_geometry_glsl[507];
static const unsigned long int particle_plot_geometry_glsl_bitcount = 4054;
extern const unsigned char particle_plot_fragment_glsl[73];
static const unsigned long int particle_plot_fragment_glsl_bitcount = 581;
extern const unsigned char shadowbuffer_vertex_glsl[106];
static const unsigned long int shadowbuffer_vertex_glsl_bitcount = 848;
extern const unsigned char shadowbuffer_fragment_glsl[25];
static const unsigned long int shadowbuffer_fragment_glsl_bitcount = 196;
extern const unsigned char secondary_vertex_glsl[240];
static const unsigned long int secondary_vertex_glsl_bitcount = 1919;
extern const unsigned char secondary_fragment_glsl[7297];
static const unsigned long int secondary_fragment_glsl_bitcount = 58376;
extern const unsigned char motionblur_vertex_glsl[128];
static const unsigned long int motionblur_vertex_glsl_bitcount = 1021;
extern const unsigned char motionblur_fragment_glsl[400];
static const unsigned long int motionblur_fragment_glsl_bitcount = 3195;
extern const unsigned char light_vertex_glsl[217];
static const unsigned long int light_vertex_glsl_bitcount = 1735;
extern const unsigned char light_fragment_glsl[450];
static const unsigned long int light_fragment_glsl_bitcount = 3599;
extern const unsigned char primary_vertex_glsl[532];
static const unsigned long int primary_vertex_glsl_bitcount = 4256;
extern const unsigned char primary_geometry_glsl[1370];
static const unsigned long int primary_geometry_glsl_bitcount = 10957;
extern const unsigned char primary_fragment_glsl[2676];
static const unsigned long int primary_fragment_glsl_bitcount = 21405;
extern const unsigned char subdiv_vertex_glsl[109];
static const unsigned long int subdiv_vertex_glsl_bitcount = 872;
extern const unsigned char subdiv_geometry_glsl[1417];
static const unsigned long int subdiv_geometry_glsl_bitcount = 11331;
extern const unsigned char subdiv_fragment_glsl[1525];
static const unsigned long int subdiv_fragment_glsl_bitcount = 12198;
extern const unsigned char skybox_vertex_glsl[70];
static const unsigned long int skybox_vertex_glsl_bitcount = 560;
extern const unsigned char skybox_geometry_glsl[476];
static const unsigned long int skybox_geometry_glsl_bitcount = 3801;
extern const unsigned char skybox_fragment_glsl[96];
static const unsigned long int skybox_fragment_glsl_bitcount = 767;
extern const unsigned char staticparticle_vertex_glsl[128];
static const unsigned long int staticparticle_vertex_glsl_bitcount = 1021;
extern const unsigned char staticparticle_fragment_glsl[777];
static const unsigned long int staticparticle_fragment_glsl_bitcount = 6213;
extern const unsigned char crystal_vertex_glsl[202];
static const unsigned long int crystal_vertex_glsl_bitcount = 1611;
extern const unsigned char crystal_geometry_glsl[902];
static const unsigned long int crystal_geometry_glsl_bitcount = 7216;
extern const unsigned char crystal_fragment_glsl[253];
static const unsigned long int crystal_fragment_glsl_bitcount = 2019;
extern const unsigned char sphere_vertex_glsl[202];
static const unsigned long int sphere_vertex_glsl_bitcount = 1611;
extern const unsigned char sphere_geometry_glsl[902];
static const unsigned long int sphere_geometry_glsl_bitcount = 7216;
extern const unsigned char sphere_fragment_glsl[121];
static const unsigned long int sphere_fragment_glsl_bitcount = 967;
extern const unsigned char skyscraper_vertex_glsl[186];
static const unsigned long int skyscraper_vertex_glsl_bitcount = 1487;
extern const unsigned char skyscraper_geometry_glsl[886];
static const unsigned long int skyscraper_geometry_glsl_bitcount = 7082;
extern const unsigned char skyscraper_fragment_glsl[695];
static const unsigned long int skyscraper_fragment_glsl_bitcount = 5559;
extern const unsigned char chunk_vertex_glsl[507];
static const unsigned long int chunk_vertex_glsl_bitcount = 4053;
extern const unsigned char chunk_geometry_glsl[1270];
static const unsigned long int chunk_geometry_glsl_bitcount = 10156;
extern const unsigned char chunk_fragment_glsl[1932];
static const unsigned long int chunk_fragment_glsl_bitcount = 15452;
extern const unsigned char chunk_shadow_vertex_glsl[326];
static const unsigned long int chunk_shadow_vertex_glsl_bitcount = 2605;
extern const unsigned char chunk_shadow_fragment_glsl[25];
static const unsigned long int chunk_shadow_fragment_glsl_bitcount = 196;
extern const unsigned char chunk_update_vertex_glsl[128];
static const unsigned long int chunk_update_vertex_glsl_bitcount = 1021;
extern const unsigned char chunk_update_fragment_glsl[1820];
static const unsigned long int chunk_update_fragment_glsl_bitcount = 14560;
extern const unsigned char sky_vertex_glsl[373];
static const unsigned long int sky_vertex_glsl_bitcount = 2978;
extern const unsigned char sky_fragment_glsl[342];
static const unsigned long int sky_fragment_glsl_bitcount = 2733;
extern const unsigned char raytrace_vertex_glsl[789];
static const unsigned long int raytrace_vertex_glsl_bitcount = 6310;
extern const unsigned char raytrace_geometry_glsl[750];
static const unsigned long int raytrace_geometry_glsl_bitcount = 5998;
extern const unsigned char raytrace_fragment_glsl[3347];
static const unsigned long int raytrace_fragment_glsl_bitcount = 26776;
extern const unsigned char blur_vertex_glsl[126];
static const unsigned long int blur_vertex_glsl_bitcount = 1006;
extern const unsigned char blur_fragment_glsl[410];
static const unsigned long int blur_fragment_glsl_bitcount = 3273;
extern const unsigned char bloom_vertex_glsl[126];
static const unsigned long int bloom_vertex_glsl_bitcount = 1006;
extern const unsigned char bloom_fragment_glsl[235];
static const unsigned long int bloom_fragment_glsl_bitcount = 1879;
extern const unsigned char post_vertex_glsl[128];
static const unsigned long int post_vertex_glsl_bitcount = 1021;
extern const unsigned char post_fragment_glsl[407];
static const unsigned long int post_fragment_glsl_bitcount = 3249;
extern const unsigned char paper_vertex_glsl[128];
static const unsigned long int paper_vertex_glsl_bitcount = 1021;
extern const unsigned char paper_fragment_glsl[1422];
static const unsigned long int paper_fragment_glsl_bitcount = 11371;
extern const unsigned char info_vertex_glsl[144];
static const unsigned long int info_vertex_glsl_bitcount = 1149;
extern const unsigned char info_fragment_glsl[159];
static const unsigned long int info_fragment_glsl_bitcount = 1268;
