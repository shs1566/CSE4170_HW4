
// The object modelling tasks performed by this file are usually done 
// by reading a scene configuration file or through a help of graphics user interface!!!

#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

/* Camera Struct */
typedef struct _CAMERA {
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;
	float fov_y, aspect_ratio, near_clip, far_clip;
	int move_status;
} CAMERA;

/* Define for Camera */
#define CAM_TRANSLATION_SPEED 5.0f
#define CAM_ROTATION_SPEED 1.0f
#define CAM_ZOOM_SPEED 0.1f

#define CAM_AXIS_LENGTH 10

CAMERA camera_world;
CAMERA camera_cctv1, camera_cctv2, camera_cctv3;
CAMERA camera_down, camera_front, camera_side;
CAMERA camera_tiger;

CAMERA *selected;

typedef struct _material {
	glm::vec4 emission, ambient, diffuse, specular;
	GLfloat exponent;
} Material;

typedef struct _Light_Parameters {
	int light_on;
	float position[4];
	float ambient_color[4], diffuse_color[4], specular_color[4];
	float spot_direction[3];
	float spot_exponent;
	float spot_cutoff_angle;
	float light_attenuation_factors[4]; // produce this effect only if .w != 0.0f
} Light_Parameters;

typedef struct _loc_LIGHT_Parameters {
	GLint light_on;
	GLint position;
	GLint ambient_color, diffuse_color, specular_color;
	GLint spot_direction;
	GLint spot_exponent;
	GLint spot_cutoff_angle;
	GLint light_attenuation_factors;
} loc_light_Parameters;

typedef struct _loc_Material_Parameters {
	GLint ambient_color, diffuse_color, specular_color, emissive_color;
	GLint specular_exponent;
} loc_Material_Parameters;

#define NUMBER_OF_LIGHT_SUPPORTED 6

loc_Material_Parameters loc_material;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
GLint loc_global_ambient_color;

Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];
#define N_MAX_GEOM_COPIES 5

typedef struct _Object {
	char filename[512];

	GLenum front_face_mode; // clockwise or counter-clockwise
	int n_triangles;

	int n_fields; // 3 floats for vertex, 3 floats for normal, and 2 floats for texcoord
	GLfloat *vertices; // pointer to vertex array data
	GLfloat xmin, xmax, ymin, ymax, zmin, zmax; // bounding box <- compute this yourself

	GLuint VBO, VAO; // Handles to vertex buffer object and vertex array object

	int n_geom_instances;
	glm::mat4 ModelMatrix[N_MAX_GEOM_COPIES];
	Material material[N_MAX_GEOM_COPIES];
} Object;

#define N_MAX_STATIC_OBJECTS		11
Object static_objects[N_MAX_STATIC_OBJECTS]; // allocage memory dynamically every time it is needed rather than using a static array
int n_static_objects = 0;

#define OBJ_BUILDING		0
#define OBJ_TABLE			1
#define OBJ_LIGHT			2
#define OBJ_TEAPOT			3
#define OBJ_NEW_CHAIR		4
#define OBJ_FRAME			5
#define OBJ_NEW_PICTURE		6
#define OBJ_COW				7
#define OBJ_CAR_BODY		8
#define OBJ_CAR_WHEEL		9
#define OBJ_CAR_NUT			10	

Material material_screen;

int read_geometry(GLfloat **object, int bytes_per_primitive, char *filename) {
	int n_triangles;
	FILE *fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open the object file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float *)malloc(n_triangles*bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Error: cannot allocate memory for the geometry file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(*object, bytes_per_primitive, n_triangles, fp); // assume the data file has no faults.
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void compute_AABB(Object *obj_ptr) {
	// Do it yourself.
}
	 
void prepare_geom_of_static_object(Object *obj_ptr) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = obj_ptr->n_fields * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	obj_ptr->n_triangles = read_geometry(&(obj_ptr->vertices), n_bytes_per_triangle, obj_ptr->filename);

	// Initialize vertex buffer object.
	glGenBuffers(1, &(obj_ptr->VBO));

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glBufferData(GL_ARRAY_BUFFER, obj_ptr->n_triangles*n_bytes_per_triangle, obj_ptr->vertices, GL_STATIC_DRAW);

	compute_AABB(obj_ptr);
	free(obj_ptr->vertices);

	// Initialize vertex array object.
	glGenVertexArrays(1, &(obj_ptr->VAO));
	glBindVertexArray(obj_ptr->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void define_static_objects(void) {

	// building
	strcpy(static_objects[OBJ_BUILDING].filename, "Data/Building1_vnt.geom");
	static_objects[OBJ_BUILDING].n_fields = 8;

	static_objects[OBJ_BUILDING].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_BUILDING]));

	static_objects[OBJ_BUILDING].n_geom_instances = 1;

    static_objects[OBJ_BUILDING].ModelMatrix[0] = glm::mat4(1.0f);
	
	static_objects[OBJ_BUILDING].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].diffuse = glm::vec4(0.54f, 0.89f, 0.63f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].specular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].exponent = 128.0f*0.1f;

	// table
	strcpy(static_objects[OBJ_TABLE].filename, "Data/Table_vn.geom");
	static_objects[OBJ_TABLE].n_fields = 6;

	static_objects[OBJ_TABLE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TABLE]));

	static_objects[OBJ_TABLE].n_geom_instances = 3;

	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(157.0f, 76.5f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[0], 
		glm::vec3(0.5f, 0.5f, 0.5f));

	static_objects[OBJ_TABLE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[0].ambient = glm::vec4(0.1f, 0.3f, 0.1f, 1.0f);
	static_objects[OBJ_TABLE].material[0].diffuse = glm::vec4(0.4f, 0.6f, 0.3f, 1.0f);
	static_objects[OBJ_TABLE].material[0].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[0].exponent = 15.0f;

	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(198.0f, 120.0f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[1],
		glm::vec3(0.8f, 0.6f, 0.6f));

	static_objects[OBJ_TABLE].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[1].ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	static_objects[OBJ_TABLE].material[1].diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	static_objects[OBJ_TABLE].material[1].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[1].exponent = 128.0f*0.078125f;

	static_objects[OBJ_TABLE].ModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(188.0f, 25.0f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[2] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[2],
		glm::vec3(0.8f, 0.6f, 0.6f));

	static_objects[OBJ_TABLE].material[2].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[2].ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	static_objects[OBJ_TABLE].material[2].diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	static_objects[OBJ_TABLE].material[2].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[2].exponent = 128.0f*0.078125f;

	// Light

	light[0].light_on = 1;

	strcpy(static_objects[OBJ_LIGHT].filename, "Data/Light_vn.geom");
	static_objects[OBJ_LIGHT].n_fields = 6;

	static_objects[OBJ_LIGHT].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(static_objects + OBJ_LIGHT);

	static_objects[OBJ_LIGHT].n_geom_instances = 5;

	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 100.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].exponent = 128.0f*0.4f;

	/* LIGHT 1 : Common light */
	light[1].light_on = 1;
	light[1].position[0] = 120.0f; light[1].position[1] = 100.0f; light[1].position[2] = 75.0f; light[1].position[3] = 1.0f;
	light[1].ambient_color[0] = 0.1f; light[1].ambient_color[1] = 0.1f; light[1].ambient_color[2] = 0.1f; light[1].ambient_color[3] = 1.0f;
	light[1].diffuse_color[0] = 0.5f; light[1].diffuse_color[1] = 0.5f; light[1].diffuse_color[2] = 0.5f; light[1].diffuse_color[3] = 1.0f;
	light[1].specular_color[0] = 0.2f; light[1].specular_color[1] = 0.2f; light[1].specular_color[2] = 0.2f; light[1].specular_color[3] = 1.0f;
	light[1].spot_direction[0] = 0.0f; light[1].spot_direction[1] = 0.0f; light[1].spot_direction[2] = -1.0f;
	light[1].spot_cutoff_angle = 180.0f;
	light[1].spot_exponent = 0.0f;
	light[1].light_attenuation_factors[0] = 1.0f; light[1].light_attenuation_factors[1] = 0.0f; light[1].light_attenuation_factors[2] = 0.0f; light[0].light_attenuation_factors[3] = 0.0f;

	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(80.0f, 47.5f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[1],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].exponent = 128.0f*0.4f;

	/* LIGHT 2 : Red light */
	light[2].light_on = 1;
	light[2].position[0] = 75.0f; light[2].position[1] = 47.5f; light[2].position[2] = 75.0f; light[2].position[3] = 1.0f;
	light[2].ambient_color[0] = 0.2f; light[2].ambient_color[1] = 0.2f; light[2].ambient_color[2] = 0.2f; light[2].ambient_color[3] = 1.0f;
	light[2].diffuse_color[0] = 0.5f; light[2].diffuse_color[1] = 0.0f; light[2].diffuse_color[2] = 0.0f; light[2].diffuse_color[3] = 1.0f;
	light[2].specular_color[0] = 0.1f; light[2].specular_color[1] = 0.0f; light[2].specular_color[2] = 0.0f; light[2].specular_color[3] = 1.0f;
	light[2].spot_direction[0] = 0.0f; light[2].spot_direction[1] = 0.0f; light[2].spot_direction[2] = -1.0f;
	light[2].spot_cutoff_angle = 10.0f;
	light[2].spot_exponent = 0.0f;
	light[2].light_attenuation_factors[0] = 1.0f; light[2].light_attenuation_factors[1] = 1.0f; light[2].light_attenuation_factors[2] = 1.0f; light[2].light_attenuation_factors[3] = 0.0f;

	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 90.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[2],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[2].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].exponent = 128.0f*0.4f;

	/* LIGHT 3 : Green light */
	light[3].light_on = 1;
	light[3].position[0] = 40.0f; light[3].position[1] = 90.0f; light[3].position[2] = 75.0f; light[3].position[3] = 1.0f;
	light[3].ambient_color[1] = 0.2f; light[3].ambient_color[1] = 0.2f; light[3].ambient_color[2] = 0.2f; light[3].ambient_color[3] = 1.0f;
	light[3].diffuse_color[1] = 0.0f; light[3].diffuse_color[1] = 0.5f; light[3].diffuse_color[2] = 0.0f; light[3].diffuse_color[3] = 1.0f;
	light[3].specular_color[1] = 0.0f; light[3].specular_color[1] = 0.1f; light[3].specular_color[1] = 0.0f; light[3].specular_color[3] = 1.0f;
	light[3].spot_direction[1] = 0.0f; light[3].spot_direction[1] = 0.0f; light[3].spot_direction[2] = -1.0f;
	light[3].spot_cutoff_angle = 10.0f;
	light[3].spot_exponent = 0.0f;
	light[3].light_attenuation_factors[0] = 1.0f; light[3].light_attenuation_factors[1] = 1.0f; light[3].light_attenuation_factors[2] = 1.0f; light[3].light_attenuation_factors[3] = 0.0f;

	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::translate(glm::mat4(1.0f), glm::vec3(190.0f, 30.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[3],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[3].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].exponent = 128.0f*0.4f;

	/* LIGHT 4 : Blue light */
	light[4].light_on = 1;
	light[4].position[0] = 190.0f; light[4].position[1] = 30.0f; light[4].position[2] = 75.0f; light[4].position[3] = 1.0f;
	light[4].ambient_color[1] = 0.2f; light[4].ambient_color[1] = 0.2f; light[4].ambient_color[2] = 0.2f; light[4].ambient_color[3] = 1.0f;
	light[4].diffuse_color[1] = 0.0f; light[4].diffuse_color[1] = 0.0f; light[4].diffuse_color[2] = 0.5f; light[4].diffuse_color[3] = 1.0f;
	light[4].specular_color[1] = 0.0f; light[4].specular_color[1] = 0.0f; light[4].specular_color[1] = 0.5f; light[4].specular_color[3] = 1.0f;
	light[4].spot_direction[1] = 0.0f; light[4].spot_direction[1] = 0.0f; light[4].spot_direction[2] = -1.0f;
	light[4].spot_cutoff_angle = 10.0f;
	light[4].spot_exponent = 0.0f;
	light[4].light_attenuation_factors[0] = 1.0f; light[4].light_attenuation_factors[1] = 1.0f; light[4].light_attenuation_factors[2] = 1.0f; light[4].light_attenuation_factors[3] = 0.0f;

	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::translate(glm::mat4(1.0f), glm::vec3(210.0f, 112.5f, 49.0));
	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[4],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[4].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].exponent = 128.0f*0.4f;

	/* LIGHT 5: Special light */
	light[5].light_on = 1;
	light[5].position[0] = 200.0f; light[5].position[1] = 112.5f; light[5].position[2] = 75.0f; light[5].position[3] = 1.0f;
	light[5].ambient_color[1] = 0.1f; light[5].ambient_color[1] = 0.1f; light[5].ambient_color[2] = 0.1f; light[5].ambient_color[3] = 1.0f;
	light[5].diffuse_color[1] = 1.0f; light[5].diffuse_color[1] = 1.0f; light[5].diffuse_color[2] = 1.0f; light[5].diffuse_color[3] = 1.0f;
	light[5].specular_color[1] = 1.0f; light[5].specular_color[1] = 1.0f; light[5].specular_color[1] = 1.0f; light[5].specular_color[3] = 1.0f;
	light[5].spot_direction[1] = 0.0f; light[5].spot_direction[1] = 0.0f; light[5].spot_direction[2] = -1.0f;
	light[5].spot_cutoff_angle = 20.0f;
	light[5].spot_exponent = 0.0f;
	light[5].light_attenuation_factors[0] = 1.0f; light[5].light_attenuation_factors[1] = 1.0f; light[5].light_attenuation_factors[2] = 1.0f; light[5].light_attenuation_factors[3] = 0.0f;

	// teapot
	strcpy(static_objects[OBJ_TEAPOT].filename, "Data/Teapotn_vn.geom");
	static_objects[OBJ_TEAPOT].n_fields = 6;

	static_objects[OBJ_TEAPOT].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TEAPOT]));

	static_objects[OBJ_TEAPOT].n_geom_instances = 2;

	static_objects[OBJ_TEAPOT].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 11.0f));
	static_objects[OBJ_TEAPOT].ModelMatrix[0] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[0],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].exponent = 128.0f*0.6;

	static_objects[OBJ_TEAPOT].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(188.0f, 25.0f, 10.0f));
	static_objects[OBJ_TEAPOT].ModelMatrix[1] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[1],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].exponent = 128.0f*0.6;

	// new_chair
	strcpy(static_objects[OBJ_NEW_CHAIR].filename, "Data/new_chair_vnt.geom");
	static_objects[OBJ_NEW_CHAIR].n_fields = 8;

	static_objects[OBJ_NEW_CHAIR].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_CHAIR]));

	static_objects[OBJ_NEW_CHAIR].n_geom_instances = 2;

	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 110.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_CHAIR].ModelMatrix[0],
		glm::vec3(0.8f, 0.8f, 0.8f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_CHAIR].ModelMatrix[0],
		180.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_NEW_CHAIR].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].exponent = 128.0f*0.078125f;

	static_objects[OBJ_NEW_CHAIR].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(73.0f, 105.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[1] = glm::scale(static_objects[OBJ_NEW_CHAIR].ModelMatrix[1],
		glm::vec3(0.8f, 0.8f, 0.8f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[1] = glm::rotate(static_objects[OBJ_NEW_CHAIR].ModelMatrix[1],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_NEW_CHAIR].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].exponent = 128.0f*0.078125f;

	// frame
	strcpy(static_objects[OBJ_FRAME].filename, "Data/Frame_vn.geom");
	static_objects[OBJ_FRAME].n_fields = 6;

	static_objects[OBJ_FRAME].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_FRAME]));

	static_objects[OBJ_FRAME].n_geom_instances = 2;

	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(188.0f, 116.0f, 30.0f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::scale(static_objects[OBJ_FRAME].ModelMatrix[0],
		glm::vec3(0.6f, 0.6f, 0.6f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::rotate(static_objects[OBJ_FRAME].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_FRAME].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_FRAME].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_FRAME].material[0].diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	static_objects[OBJ_FRAME].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_FRAME].material[0].exponent = 128.0f*0.1f;

	static_objects[OBJ_FRAME].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(73.0f, 105.0f, 30.0f));
	static_objects[OBJ_FRAME].ModelMatrix[1] = glm::scale(static_objects[OBJ_FRAME].ModelMatrix[1],
		glm::vec3(0.6f, 0.6f, 0.6f));
	static_objects[OBJ_FRAME].ModelMatrix[1] = glm::rotate(static_objects[OBJ_FRAME].ModelMatrix[1],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_FRAME].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_FRAME].material[1].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_FRAME].material[1].diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	static_objects[OBJ_FRAME].material[1].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_FRAME].material[1].exponent = 128.0f*0.1f;


	// new_picture
	strcpy(static_objects[OBJ_NEW_PICTURE].filename, "Data/new_picture_vnt.geom");
	static_objects[OBJ_NEW_PICTURE].n_fields = 8;

	static_objects[OBJ_NEW_PICTURE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_PICTURE]));

	static_objects[OBJ_NEW_PICTURE].n_geom_instances = 1;

	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(189.5f, 116.0f, 30.0f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
		glm::vec3(13.5f*0.6f, 13.5f*0.6f, 13.5f*0.6f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
		 90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_NEW_PICTURE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].exponent = 128.0f*0.6f;

	// new_picture
	strcpy(static_objects[OBJ_COW].filename, "Data/cow_vn.geom");
	static_objects[OBJ_COW].n_fields = 6;

	static_objects[OBJ_COW].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_COW]));

	static_objects[OBJ_COW].n_geom_instances = 2;

	static_objects[OBJ_COW].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(215.0f, 100.0f, 9.5f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::scale(static_objects[OBJ_COW].ModelMatrix[0],
		glm::vec3(30.0f, 30.0f, 30.0f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
 
	static_objects[OBJ_COW].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_COW].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW].material[0].exponent = 0.21794872f*0.6f;

	static_objects[OBJ_COW].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 85.0f, 9.5f));
	static_objects[OBJ_COW].ModelMatrix[1] = glm::scale(static_objects[OBJ_COW].ModelMatrix[1],
		glm::vec3(30.0f, 30.0f, 30.0f));
	static_objects[OBJ_COW].ModelMatrix[1] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[1],
		0.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_COW].ModelMatrix[1] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[1],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_COW].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_COW].material[1].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW].material[1].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW].material[1].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW].material[1].exponent = 0.21794872f*0.6f;

	n_static_objects = 8;
}

void draw_static_object(Object *obj_ptr, int instance_ID) {
	glm::mat3 ModelViewMatrixInvTrans;

	glFrontFace(obj_ptr->front_face_mode);

	ModelViewMatrix = ViewMatrix * obj_ptr->ModelMatrix[instance_ID];
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverse(glm::mat3(ModelViewMatrix));
	ModelViewMatrixInvTrans = glm::transpose(ModelViewMatrixInvTrans);

	

	glUniform4f(loc_material.ambient_color, obj_ptr->material[instance_ID].ambient.r, obj_ptr->material[instance_ID].ambient.g, obj_ptr->material[instance_ID].ambient.b, obj_ptr->material[instance_ID].ambient.a);
	glUniform4f(loc_material.diffuse_color, obj_ptr->material[instance_ID].diffuse.r, obj_ptr->material[instance_ID].diffuse.g, obj_ptr->material[instance_ID].diffuse.b, obj_ptr->material[instance_ID].diffuse.a);
	glUniform4f(loc_material.specular_color, obj_ptr->material[instance_ID].specular.r, obj_ptr->material[instance_ID].specular.g, obj_ptr->material[instance_ID].specular.b, obj_ptr->material[instance_ID].specular.a);
	glUniform4f(loc_material.emissive_color, obj_ptr->material[instance_ID].emission.r, obj_ptr->material[instance_ID].emission.g, obj_ptr->material[instance_ID].emission.b, obj_ptr->material[instance_ID].emission.a);

	glUniform3f(loc_primitive_color, obj_ptr->material[instance_ID].diffuse.r, obj_ptr->material[instance_ID].diffuse.g, obj_ptr->material[instance_ID].diffuse.b);
	
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_S, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_S, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glBindVertexArray(obj_ptr->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * obj_ptr->n_triangles);
	glBindVertexArray(0);
}

GLuint rectangle_VBO, rectangle_VAO;
GLfloat rectangle_vertices[12][3] = {  // vertices enumerated counterclockwise
	{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f }
};

void prepare_screen(void) { // Draw coordinate axes.
						   // Initialize vertex buffer object.
	glGenBuffers(1, &rectangle_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &rectangle_VAO);
	glBindVertexArray(rectangle_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_screen.ambient[0] = 1.0f;
	material_screen.ambient[1] = 0.5f;
	material_screen.ambient[2] = 0.0f;
	material_screen.ambient[3] = 0.5f;
			 
	material_screen.diffuse[0] = 0.4f;
	material_screen.diffuse[1] = 0.5f;
	material_screen.diffuse[2] = 0.4f;
	material_screen.diffuse[3] = 1.0f;
			
	material_screen.specular[0] = 0.04f;
	material_screen.specular[1] = 0.7f;
	material_screen.specular[2] = 0.04f;
	material_screen.specular[3] = 1.0f;
		
	material_screen.exponent = 2.5f;
			
	material_screen.emission[0] = 0.0f;
	material_screen.emission[1] = 0.0f;
	material_screen.emission[2] = 0.2f;
	material_screen.emission[3] = 0.8f;
}

void set_screen() {
	material_screen.ambient[0] = 0.0f;
	material_screen.ambient[1] = 0.0f;
	material_screen.ambient[2] = 0.0f;
	material_screen.ambient[3] = 1.0f;
			 
	material_screen.diffuse[0] = 0.0f;
	material_screen.diffuse[1] = 0.0f;
	material_screen.diffuse[2] = 0.0f;
	material_screen.diffuse[3] = 1.0f;
			 
	material_screen.specular[0] = 0.0f;
	material_screen.specular[1] = 0.0f;
	material_screen.specular[2] = 0.0f;
	material_screen.specular[3] = 1.0f;
			 
	material_screen.exponent = 0.0f;
			 
	material_screen.emission[0] = 0.0f;
	material_screen.emission[1] = 0.0f;
	material_screen.emission[2] = 0.0f;
	material_screen.emission[3] = 1.0f;
}

void set_material_screen(void) {
	// assume ShaderProgram_PS is used
	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 0.0f);
	glUniform3f(loc_material.ambient_color, material_screen.ambient.r, material_screen.ambient.g, material_screen.ambient.b);
	glUniform3f(loc_material.diffuse_color, material_screen.diffuse.r, material_screen.diffuse.g, material_screen.diffuse.b);
	glUniform3f(loc_material.specular_color, material_screen.specular.r, material_screen.specular.g, material_screen.specular.b);
	glUniform1f(loc_material.specular_exponent, material_screen.exponent);
	glUniform3f(loc_material.emissive_color, material_screen.emission.r, material_screen.emission.g, material_screen.emission.b);
}

void draw_screen(void) {
	glFrontFace(GL_CCW);

	glBindVertexArray(rectangle_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

GLuint VBO_axes, VAO_axes;

GLfloat vertices_axes[6][3] = {
	{ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } };

GLuint VBO_volume, VAO_volume;
GLfloat view_volume[17][3] = { {0.0f, 0.0f, 0.0f}, {-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, { -1.0f, -1.0f, -1.0f }, 
{ -2.0f, 2.0f, -2.0f }, { 2.0f, 2.0f, -2.0f }, { 2.0f, -2.0f, -2.0f }, { -2.0f, -2.0f, -2.0f }, 
{ 0.0f, 0.0f, 0.0f }, { -2.0f, 2.0f, -2.0f }, { 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, -2.0f }, 
{ 0.0f, 0.0f, 0.0f }, { 2.0f, -2.0f, -2.0f }, { 0.0f, 0.0f, 0.0f }, { -2.0f, -2.0f, -2.0f } };

GLfloat volume_color[3] = {255.0f, 255.0f, 0.0f};

void define_volume() {

	view_volume[1][0] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[1][1] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[1][2] = -camera_world.near_clip;

	view_volume[2][0] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[2][1] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[2][2] = -camera_world.near_clip;

	view_volume[3][0] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[3][1] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[3][2] = -camera_world.near_clip;

	view_volume[4][0] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[4][1] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.near_clip;
	view_volume[4][2] = -camera_world.near_clip;

	view_volume[5][0] = view_volume[10][0] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[5][1] = view_volume[10][1] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[5][2] = view_volume[10][2] = -camera_world.far_clip;

	view_volume[6][0] = view_volume[12][0] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[6][1] = view_volume[12][1] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[6][2] = view_volume[12][2] = -camera_world.far_clip;

	view_volume[7][0] = view_volume[14][0] = tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[7][1] = view_volume[14][1] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[7][2] = view_volume[14][2] = -camera_world.far_clip;

	view_volume[8][0] = view_volume[16][0] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[8][1] = view_volume[16][1] = -tan(camera_world.fov_y / 2.0f * TO_RADIAN) * camera_world.far_clip;
	view_volume[8][2] = view_volume[16][2] = -camera_world.far_clip;

	glGenBuffers(1, &VBO_volume);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_volume);
	glBufferData(GL_ARRAY_BUFFER, sizeof(view_volume), &view_volume[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_volume);
	glBindVertexArray(VAO_volume);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_volume);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_volume(void) {

 	glBindVertexArray(VAO_volume);
	glUniform3fv(loc_primitive_color, 1, volume_color);
	glDrawArrays(GL_LINE_LOOP, 1, 4);
	glUniform3fv(loc_primitive_color, 1, volume_color);
	glDrawArrays(GL_LINE_LOOP, 5, 4);

	glUniform3fv(loc_primitive_color, 1, volume_color);
	glDrawArrays(GL_LINE_LOOP, 9, 2);
	glUniform3fv(loc_primitive_color, 1, volume_color);
	glDrawArrays(GL_LINE_LOOP, 11, 2);
	glUniform3fv(loc_primitive_color, 1, volume_color);
	glDrawArrays(GL_LINE_LOOP, 13, 2);
	glUniform3fv(loc_primitive_color, 1, volume_color);
	glDrawArrays(GL_LINE_LOOP, 15, 2);
	
	glBindVertexArray(0);
}

void define_axes(void) {  
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_axes), &vertices_axes[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

#define WC_AXIS_LENGTH		60.0f
void draw_axes(void) {
	
	glBindVertexArray(VAO_axes);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}

#define N_TIGER_FRAMES 12
Object tiger[N_TIGER_FRAMES];
glm::vec4 tiger_pos[200];
#define N_TIGER_POS_FRAME 130
struct {
	int cur_frame = 0;
	float rotation_angle = 0.0f;
	int pos = 0;
	bool direction = true;
} tiger_data;

void initialize_tiger_pos() {
	float x_pos = 80.0f;
	float y_pos = 30.0f;
	float angle = 180.0f;

	for (int i = 0; i < 200; i++) {

		/* 50 frames */
		if (y_pos <= 80) {
			tiger_pos[i] = glm::vec4(x_pos, y_pos++, 0.0f, 180.0f);
		}
		/* 20 frames */
		else if (y_pos <= 100) {
			tiger_pos[i] = glm::vec4(x_pos++, y_pos++, 0.0f, angle);
			angle -= 4.5f;
		}
		/* 40 frames */
		else if (x_pos <= 140) {
			tiger_pos[i] = glm::vec4(x_pos++, y_pos, 0.0f, angle);
		}
		/* 40 frames */
		else if (y_pos <= 140) {
			tiger_pos[i] = glm::vec4(x_pos++, y_pos++, 0.0f, angle);
			angle += 4.5f;
			if (angle > 180.0f) {
				angle = 180.0f;
				x_pos--;
			}
		}
	}
}

void define_animated_tiger(void) {
	for (int i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(tiger[i].filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);

		tiger[i].n_fields = 8;
		tiger[i].front_face_mode = GL_CW;
		prepare_geom_of_static_object(&(tiger[i]));

		tiger[i].n_geom_instances = 1;

		tiger[i].ModelMatrix[0] = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));

		tiger[i].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		tiger[i].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
		tiger[i].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
		tiger[i].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
		tiger[i].material[0].exponent = 128.0f*0.21794872f;
	}

	initialize_tiger_pos();
}

const glm::vec3 first_tiger_pos = glm::vec3(0.0f, 0.0f, 15.0f);
const glm::vec3 first_tiger_uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 first_tiger_vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
const glm::vec3 first_tiger_naxis = glm::vec3(0.0f, 1.0f, 0.0f);

void draw_animated_tiger(void) {
	glm::mat3 ModelViewMatrixInvTrans;

 	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(tiger_pos[tiger_data.pos].x, tiger_pos[tiger_data.pos].y, tiger_pos[tiger_data.pos].z));
	if (tiger_data.direction)
		ModelViewMatrix = glm::rotate(ModelViewMatrix, tiger_pos[tiger_data.pos].w*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (tiger_pos[tiger_data.pos].w-180.0f)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix *= tiger[tiger_data.cur_frame].ModelMatrix[0];

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverse(glm::mat3(ModelViewMatrix));
	ModelViewMatrixInvTrans = glm::transpose(ModelViewMatrixInvTrans);

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_S, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_S, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glUniform4f(loc_material.diffuse_color, tiger[tiger_data.cur_frame].material[0].diffuse.r, tiger[tiger_data.cur_frame].material[0].diffuse.g, tiger[tiger_data.cur_frame].material[0].diffuse.b, tiger[tiger_data.cur_frame].material[0].diffuse.a);
	glUniform4f(loc_material.ambient_color, tiger[tiger_data.cur_frame].material[0].ambient.r, tiger[tiger_data.cur_frame].material[0].ambient.g, tiger[tiger_data.cur_frame].material[0].ambient.b, tiger[tiger_data.cur_frame].material[0].diffuse.a);
	glUniform4f(loc_material.specular_color, tiger[tiger_data.cur_frame].material[0].specular.r, tiger[tiger_data.cur_frame].material[0].specular.g, tiger[tiger_data.cur_frame].material[0].specular.b, tiger[tiger_data.cur_frame].material[0].diffuse.a);
	glUniform4f(loc_material.emissive_color, tiger[tiger_data.cur_frame].material[0].emission.r, tiger[tiger_data.cur_frame].material[0].emission.g, tiger[tiger_data.cur_frame].material[0].emission.b, tiger[tiger_data.cur_frame].material[0].diffuse.a);

	glUniform3f(loc_primitive_color, tiger[tiger_data.cur_frame].material[0].diffuse.r, tiger[tiger_data.cur_frame].material[0].diffuse.g, tiger[tiger_data.cur_frame].material[0].diffuse.b);

	glBindVertexArray(tiger[tiger_data.cur_frame].VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tiger[tiger_data.cur_frame].n_triangles);
	glBindVertexArray(0);

	glUseProgram(h_ShaderProgram_simple);
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();

	glm::mat4 ModelMatrix =glm:: mat4(1.0f);
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(tiger_pos[tiger_data.pos].x, tiger_pos[tiger_data.pos].y, tiger_pos[tiger_data.pos].z));
	if (tiger_data.direction)
		ModelMatrix = glm::rotate(ModelMatrix, tiger_pos[tiger_data.pos].w*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else
		ModelMatrix = glm::rotate(ModelMatrix, (tiger_pos[tiger_data.pos].w - 180.0f)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -15.0f, -2.0f));
	camera_tiger.pos = glm::vec3(ModelMatrix * glm::vec4(first_tiger_pos, 1));
	camera_tiger.uaxis = glm::vec3(ModelMatrix * glm::vec4(first_tiger_uaxis, 0));
	camera_tiger.vaxis = glm::vec3(ModelMatrix * glm::vec4(first_tiger_vaxis, 0));
	camera_tiger.naxis = glm::vec3(ModelMatrix * glm::vec4(first_tiger_naxis, 0));
}

glm::mat4 ModelMatrix_CAR_BODY, ModelMatrix_CAR_WHEEL, ModelMatrix_CAR_NUT;
float rotation_angle_car = 0.0f;
#define rad 1.7f
#define ww 1.0f

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

// codes for the 'general' triangular-mesh object
typedef enum _GEOM_OBJ_TYPE { GEOM_OBJ_TYPE_V = 0, GEOM_OBJ_TYPE_VN, GEOM_OBJ_TYPE_VNT } GEOM_OBJ_TYPE;
// GEOM_OBJ_TYPE_V: (x, y, z)
// GEOM_OBJ_TYPE_VN: (x, y, z, nx, ny, nz)
// GEOM_OBJ_TYPE_VNT: (x, y, z, nx, ny, nz, s, t)
int GEOM_OBJ_ELEMENTS_PER_VERTEX[3] = { 3, 6, 8 };

#define N_GEOMETRY_OBJECTS 3
#define GEOM_OBJ_ID_CAR_BODY 0
#define GEOM_OBJ_ID_CAR_WHEEL 1
#define GEOM_OBJ_ID_CAR_NUT 2

Material material_car;
Material material_wheel;
Material material_nut;

void set_car() {

	material_car.ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	material_car.diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	material_car.specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	material_car.exponent = 100.0f;
	material_car.emission = glm::vec4(1.0f, 0.498f, 0.831f, 1.0f);

	material_wheel.ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	material_wheel.diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	material_wheel.specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	material_wheel.exponent = 100.0f;
	material_wheel.emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	material_nut.ambient = glm::vec4(0.690f, 0.769f, 0.871f, 1.0f);
	material_nut.diffuse = glm::vec4(0.690f, 0.769f, 0.871f, 1.0f);
	material_nut.specular = glm::vec4(0.690f, 0.769f, 0.871f, 1.0f);
	material_nut.exponent = 100.0f;
	material_nut.emission = glm::vec4(0.690f, 0.769f, 0.871f, 1.0f);

}

void set_material_car() {
	glUniform3f(loc_material.ambient_color, material_car.ambient.r, material_car.ambient.g, material_car.ambient.b);
	glUniform3f(loc_material.diffuse_color, material_car.diffuse.r, material_car.diffuse.g, material_car.diffuse.b);
	glUniform3f(loc_material.specular_color, material_car.specular.r, material_car.specular.g, material_car.specular.b);
	glUniform1f(loc_material.specular_exponent, material_car.exponent);
	glUniform3f(loc_material.emissive_color, material_car.emission.r, material_car.emission.g, material_car.emission.b);

	glUniform3f(loc_primitive_color, material_car.diffuse.r, material_car.diffuse.g, material_car.diffuse.b);
}
GLuint geom_obj_VBO[N_GEOMETRY_OBJECTS];
GLuint geom_obj_VAO[N_GEOMETRY_OBJECTS];

int geom_obj_n_triangles[N_GEOMETRY_OBJECTS];
GLfloat *geom_obj_vertices[N_GEOMETRY_OBJECTS];

glm::vec4 car_pos[1000];

void cleanup_OpenGL_stuffs(void) {
	for (int i = 0; i < n_static_objects; i++) {
		glDeleteVertexArrays(1, &(static_objects[i].VAO));
		glDeleteBuffers(1, &(static_objects[i].VBO));
	}

	for (int i = 0; i < N_TIGER_FRAMES; i++) {
		glDeleteVertexArrays(1, &(tiger[i].VAO));
		glDeleteBuffers(1, &(tiger[i].VBO));
	}

	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);
}

int read_geometry_file2(GLfloat **object, const char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int i, n_triangles;
	float *flt_ptr;
	FILE *fp;

	fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the geometry file %s ...", filename);
		return -1;
	}

	fscanf(fp, "%d", &n_triangles);
	*object = (float *)malloc(3 * n_triangles*GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float));
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < 3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type]; i++)
		fscanf(fp, "%f", flt_ptr++);
	fclose(fp);

	fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	return n_triangles;
}

void prepare_geom_obj2(int geom_obj_ID, const char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int n_bytes_per_vertex;

	n_bytes_per_vertex = GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float);
	geom_obj_n_triangles[geom_obj_ID] = read_geometry_file2(&geom_obj_vertices[geom_obj_ID], filename, geom_obj_type);

	// Initialize vertex array object.
	glGenVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glGenBuffers(1, &geom_obj_VBO[geom_obj_ID]);
	glBindBuffer(GL_ARRAY_BUFFER, geom_obj_VBO[geom_obj_ID]);
	glBufferData(GL_ARRAY_BUFFER, 3 * geom_obj_n_triangles[geom_obj_ID] * n_bytes_per_vertex,
		geom_obj_vertices[geom_obj_ID], GL_STATIC_DRAW);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(geom_obj_vertices[geom_obj_ID]);
}

void draw_geom_obj(int geom_obj_ID) {

	switch (geom_obj_ID) {
	case GEOM_OBJ_ID_CAR_BODY:
		glUniform4f(loc_material.ambient_color, material_car.ambient.r, material_car.ambient.g, material_car.ambient.b, material_car.ambient.a);
		glUniform4f(loc_material.diffuse_color, material_car.diffuse.r, material_car.diffuse.g, material_car.diffuse.b, material_car.diffuse.a);
		glUniform4f(loc_material.specular_color, material_car.specular.r, material_car.specular.g, material_car.specular.b, material_car.specular.a);
		glUniform4f(loc_material.emissive_color, material_car.emission.r, material_car.emission.g, material_car.emission.b, material_car.emission.a);

		glUniform3f(loc_primitive_color, material_car.diffuse.r, material_car.diffuse.g, material_car.diffuse.b);
		break;

	case GEOM_OBJ_ID_CAR_WHEEL:
		glUniform4f(loc_material.ambient_color, material_wheel.ambient.r, material_wheel.ambient.g, material_wheel.ambient.b, material_wheel.ambient.a);
		glUniform4f(loc_material.diffuse_color, material_wheel.diffuse.r, material_wheel.diffuse.g, material_wheel.diffuse.b, material_wheel.diffuse.a);
		glUniform4f(loc_material.specular_color, material_wheel.specular.r, material_wheel.specular.g, material_wheel.specular.b, material_wheel.specular.a);
		glUniform4f(loc_material.emissive_color, material_wheel.emission.r, material_wheel.emission.g, material_wheel.emission.b, material_wheel.emission.a);

		glUniform3f(loc_primitive_color, material_wheel.diffuse.r, material_wheel.diffuse.g, material_wheel.diffuse.b);
		break;

	case GEOM_OBJ_ID_CAR_NUT:
		glUniform4f(loc_material.ambient_color, material_nut.ambient.r, material_nut.ambient.g, material_nut.ambient.b, material_nut.ambient.a);
		glUniform4f(loc_material.diffuse_color, material_nut.diffuse.r, material_nut.diffuse.g, material_nut.diffuse.b, material_nut.diffuse.a);
		glUniform4f(loc_material.specular_color, material_nut.specular.r, material_nut.specular.g, material_nut.specular.b, material_nut.specular.a);
		glUniform4f(loc_material.emissive_color, material_nut.emission.r, material_nut.emission.g, material_nut.emission.b, material_nut.emission.a);

		glUniform3f(loc_primitive_color, material_car.diffuse.r, material_car.diffuse.g, material_car.diffuse.b);
		break;
	}
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * geom_obj_n_triangles[geom_obj_ID]);
	glBindVertexArray(0);


}

int car_frame = 0;
float wheel_rotate = 0.0f;

#define N_CAR_FRAME 201

void initialize_car_pos() {
	float x_pos = 202.0f;
	float y_pos = 50.0f;
	float z_pos = 5.0f;
	float angle = 90.0f;
	int cnt = 0;

	for (int i = 0; i < 24; i ++) {
		car_pos[cnt++] = glm::vec4(x_pos, y_pos+=2, z_pos, angle);
	}

	for (int i = 0; i < 15; i ++) {
		glm::mat4 Matrix;
		Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x_pos, y_pos, z_pos));
		Matrix = glm::rotate(Matrix, angle * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		Matrix = glm::translate(Matrix, glm::vec3(-x_pos, -y_pos, -z_pos));
		car_pos[cnt++] = glm::vec4(glm::vec3(Matrix * glm::vec4(x_pos, y_pos, z_pos, 1.0f)), angle);
		angle += 6.0f;
	}

	x_pos = car_pos[cnt - 1].x;
	y_pos = car_pos[cnt - 1].y;

	for (int i = 0; i < 40; i++) {
		car_pos[cnt++] = glm::vec4(x_pos -= 3, y_pos, z_pos, angle);
	}

	for (int i = 0; i < 45; i++) {
		glm::mat4 Matrix;
		Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x_pos, y_pos, z_pos));
		Matrix = glm::rotate(Matrix, angle * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		Matrix = glm::translate(Matrix, glm::vec3(-x_pos, -y_pos, -z_pos));
		car_pos[cnt++] = glm::vec4(glm::vec3(Matrix * glm::vec4(x_pos, y_pos, z_pos, 1.0f)), angle);
		angle += 4.0f;
	}

	for (int i = 0; i < 40; i++) {
		car_pos[cnt++] = glm::vec4(x_pos += 3, y_pos, z_pos, angle);
	}

	for (int i = 0; i < 15; i++) {
		glm::mat4 Matrix;
		Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x_pos, y_pos, z_pos));
		Matrix = glm::rotate(Matrix, angle * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		Matrix = glm::translate(Matrix, glm::vec3(-x_pos, -y_pos, -z_pos));
		car_pos[cnt++] = glm::vec4(glm::vec3(Matrix * glm::vec4(x_pos, y_pos, z_pos, 1.0f)), angle);
		angle -= 6.0f;
	}

	for (int i = 0; i < 24; i++) {
		car_pos[cnt++] = glm::vec4(x_pos, y_pos -= 2, z_pos, angle);
	}
}

void draw_wheel_and_nut() {
	// angle is used in Hierarchical_Car_Correct later
	int i;
	glm::mat3 ModelViewMatrixInvTrans;

	set_car();
	set_material_car();

	draw_geom_obj(GEOM_OBJ_ID_CAR_WHEEL); // draw wheel

	for (i = 0; i < 5; i++) {
		ModelMatrix_CAR_NUT = glm::rotate(ModelMatrix_CAR_WHEEL, TO_RADIAN*72.0f*i, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_CAR_NUT = glm::translate(ModelMatrix_CAR_NUT, glm::vec3(rad - 0.5f, 0.0f, ww));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelMatrix_CAR_NUT;
		ModelViewProjectionMatrix = ProjectionMatrix * ModelMatrix_CAR_NUT;
		ModelViewMatrixInvTrans = glm::inverse(glm::mat3(ModelMatrix_CAR_NUT));

		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_S, 1, GL_FALSE, &ModelMatrix_CAR_NUT[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_S, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

		//glUniform3f(loc_primitive_color, 0.690f, 0.769f, 0.871f); // color name: LightSteelBlue
		draw_geom_obj(GEOM_OBJ_ID_CAR_NUT); // draw i-th nut
	}
}

void draw_car_dummy(void) {

	glm::mat3 ModelViewMatrixInvTrans;

	draw_geom_obj(GEOM_OBJ_ID_CAR_BODY); // draw body

	glLineWidth(2.0f);
	draw_axes(); // draw MC axes of body
	glLineWidth(1.0f);

	ModelMatrix_CAR_WHEEL = glm::translate(ModelViewMatrix, glm::vec3(-3.9f, -3.5f, 4.5f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, wheel_rotate*TO_RADIAN, glm::vec3(0.0f, 0.0f, 0.1f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverse(glm::mat3(ModelMatrix_CAR_WHEEL));
	ModelViewMatrixInvTrans = glm::transpose(ModelViewMatrixInvTrans);

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_S, 1, GL_FALSE, &ModelMatrix_CAR_WHEEL[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_S, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut();  // draw wheel 0

	ModelMatrix_CAR_WHEEL = glm::translate(ModelViewMatrix, glm::vec3(3.9f, -3.5f, 4.5f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, wheel_rotate*TO_RADIAN, glm::vec3(0.0f, 0.0f, 0.1f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverse(glm::mat3(ModelMatrix_CAR_WHEEL));
	ModelViewMatrixInvTrans = glm::transpose(ModelViewMatrixInvTrans);

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_S, 1, GL_FALSE, &ModelMatrix_CAR_WHEEL[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_S, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut();  // draw wheel 1

	ModelMatrix_CAR_WHEEL = glm::translate(ModelViewMatrix, glm::vec3(-3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, wheel_rotate*TO_RADIAN, glm::vec3(0.0f, 0.0f, 0.1f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverse(glm::mat3(ModelMatrix_CAR_WHEEL));
	ModelViewMatrixInvTrans = glm::transpose(ModelViewMatrixInvTrans);

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_S, 1, GL_FALSE, &ModelMatrix_CAR_WHEEL[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_S, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut();  // draw wheel 2

	ModelMatrix_CAR_WHEEL = glm::translate(ModelViewMatrix, glm::vec3(3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, wheel_rotate*TO_RADIAN, glm::vec3(0.0f, 0.0f, 0.1f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverse(glm::mat3(ModelMatrix_CAR_WHEEL));
	ModelViewMatrixInvTrans = glm::transpose(ModelViewMatrixInvTrans);

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_S, 1, GL_FALSE, &ModelMatrix_CAR_WHEEL[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_S, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut();  // draw wheel 3
}

void free_geom_obj(int geom_obj_ID) {
	glDeleteVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glDeleteBuffers(1, &geom_obj_VBO[geom_obj_ID]);
}