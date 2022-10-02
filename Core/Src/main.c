/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MIN 1187
#define MAX 2654
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t bms_update;
uint16_t temp_in[4];
uint16_t temp_out[4];
uint8_t serial_out[154 + 8 + 4];
uint8_t BMS_REQUEST_DATA[10] = {"\x7E\xA1\x01\x00\x00\xBE\x18\x55\xAA\x55"}; // Requests 144 byte
uint8_t bms_rx_buffer[154];
uint16_t TEMP_TABLE[] = {
        1549, 1585, 1621, 1658, 1694, 1730, 1767, 1803, 1840, 1876, 1913, 1949, 1986, 2022, 2059, 2096,
        2132, 2169, 2206, 2242, 2279, 2316, 2353, 2390, 2426, 2463, 2500, 2537, 2574, 2611, 2648, 2685,
        2722, 2759, 2796, 2834, 2871, 2908, 2945, 2982, 3020, 3057, 3094, 3131, 3167, 3203, 3239, 3275,
        3311, 3347, 3383, 3419, 3456, 3492, 3528, 3564, 3600, 3637, 3673, 3709, 3746, 3782, 3818, 3855,
        3891, 3928, 3964, 4001, 4037, 4074, 4111, 4147, 4184, 4220, 4257, 4294, 4331, 4367, 4404, 4441,
        4478, 4515, 4552, 4589, 4626, 4663, 4700, 4737, 4774, 4811, 4848, 4885, 4922, 4960, 4997, 5034,
        5071, 5109, 5146, 5183, 5221, 5258, 5296, 5333, 5371, 5408, 5446, 5483, 5521, 5558, 5596, 5634,
        5671, 5709, 5747, 5785, 5823, 5860, 5898, 5936, 5974, 6012, 6050, 6088, 6126, 6164, 6202, 6240,
        6275, 6310, 6345, 6380, 6415, 6450, 6485, 6519, 6554, 6590, 6625, 6660, 6695, 6730, 6765, 6800,
        6835, 6871, 6906, 6941, 6976, 7012, 7047, 7083, 7118, 7153, 7189, 7224, 7260, 7295, 7331, 7367,
        7402, 7438, 7473, 7509, 7545, 7581, 7616, 7652, 7688, 7724, 7760, 7796, 7832, 7867, 7903, 7939,
        7976, 8012, 8048, 8084, 8120, 8156, 8192, 8228, 8265, 8301, 8337, 8374, 8410, 8446, 8483, 8519,
        8556, 8592, 8629, 8665, 8702, 8738, 8775, 8812, 8848, 8885, 8922, 8958, 8995, 9032, 9069, 9106,
        9143, 9180, 9217, 9253, 9290, 9328, 9364, 9400, 9435, 9470, 9505, 9541, 9576, 9612, 9647, 9682,
        9718, 9753, 9789, 9824, 9860, 9896, 9931, 9967, 10003, 10038, 10074, 10110, 10146, 10181, 10217, 10253,
        10289, 10325, 10361, 10397, 10433, 10469, 10505, 10541, 10577, 10613, 10649, 10686, 10722, 10758, 10794, 10831,
        10867, 10903, 10940, 10976, 11013, 11049, 11085, 11122, 11159, 11195, 11232, 11268, 11305, 11342, 11378, 11415,
        11452, 11489, 11526, 11562, 11599, 11636, 11673, 11710, 11747, 11784, 11821, 11858, 11895, 11932, 11970, 12007,
        12044, 12081, 12119, 12156, 12193, 12231, 12268, 12305, 12343, 12380, 12418, 12455, 12492, 12528, 12564, 12600,
        12636, 12672, 12708, 12744, 12780, 12816, 12852, 12888, 12924, 12960, 12996, 13032, 13069, 13105, 13141, 13177,
        13214, 13250, 13286, 13323, 13359, 13396, 13432, 13469, 13505, 13542, 13579, 13615, 13652, 13689, 13725, 13762,
        13799, 13836, 13872, 13909, 13946, 13983, 14020, 14057, 14094, 14131, 14168, 14205, 14242, 14280, 14317, 14354,
        14391, 14428, 14466, 14503, 14540, 14578, 14615, 14653, 14690, 14728, 14765, 14803, 14840, 14878, 14916, 14953,
        14991, 15029, 15067, 15104, 15142, 15180, 15218, 15256, 15294, 15332, 15370, 15408, 15446, 15484, 15522, 15560,
        15599, 15634, 15670, 15705, 15741, 15776, 15812, 15847, 15883, 15919, 15954, 15990, 16026, 16062, 16097, 16133,
        16169, 16205, 16241, 16277, 16313, 16349, 16385, 16421, 16457, 16493, 16529, 16565, 16601, 16637, 16674, 16710,
        16746, 16783, 16819, 16855, 16892, 16928, 16965, 17001, 17038, 17074, 17111, 17148, 17184, 17221, 17258, 17294,
        17331, 17368, 17405, 17442, 17478, 17515, 17552, 17589, 17626, 17663, 17700, 17738, 17775, 17812, 17849, 17886,
        17923, 17961, 17998, 18035, 18073, 18110, 18148, 18185, 18223, 18260, 18298, 18335, 18373, 18410, 18448, 18486,
        18524, 18561, 18599, 18637, 18675, 18713, 18750, 18786, 18822, 18859, 18895, 18932, 18968, 19005, 19041, 19078,
        19114, 19151, 19188, 19224, 19261, 19298, 19335, 19372, 19409, 19445, 19482, 19519, 19556, 19593, 19630, 19667,
        19705, 19742, 19779, 19816, 19853, 19891, 19928, 19965, 20003, 20040, 20077, 20115, 20152, 20190, 20227, 20265,
        20303, 20340, 20378, 20416, 20453, 20491, 20529, 20567, 20605, 20642, 20680, 20718, 20756, 20794, 20832, 20870,
        20909, 20947, 20985, 21023, 21061, 21100, 21138, 21176, 21215, 21253, 21291, 21330, 21368, 21407, 21446, 21484,
        21523, 21561, 21600, 21639, 21678, 21716, 21755, 21794, 21833, 21870, 21907, 21944, 21981, 22018, 22055, 22092,
        22129, 22166, 22204, 22241, 22278, 22315, 22352, 22390, 22427, 22464, 22502, 22539, 22577, 22614, 22652, 22689,
        22727, 22764, 22802, 22840, 22877, 22915, 22953, 22991, 23029, 23066, 23104, 23142, 23180, 23218, 23256, 23294,
        23333, 23371, 23409, 23447, 23485, 23524, 23562, 23600, 23639, 23677, 23715, 23754, 23792, 23831, 23869, 23908,
        23947, 23985, 24024, 24063, 24102, 24140, 24179, 24218, 24257, 24296, 24335, 24374, 24413, 24452, 24491, 24530,
        24569, 24609, 24648, 24687, 24727, 24766, 24805, 24845, 24884, 24924, 24963, 25001, 25039, 25077, 25116, 25154,
        25192, 25230, 25268, 25307, 25345, 25383, 25422, 25460, 25499, 25537, 25576, 25614, 25653, 25692, 25730, 25769,
        25808, 25847, 25885, 25924, 25963, 26002, 26041, 26080, 26119, 26158, 26197, 26236, 26276, 26315, 26354, 26393,
        26433, 26472, 26511, 26550, 26588, 26627, 26666, 26704, 26743, 26781, 26820, 26859, 26897, 26936, 26975, 27014,
        27053, 27092, 27131, 27170, 27209, 27248, 27287, 27326, 27365, 27404, 27443, 27483, 27522, 27561, 27601, 27640,
        27680, 27719, 27759, 27798, 27838, 27877, 27917, 27957, 27996, 28036, 28076, 28115, 28154, 28193, 28232, 28271,
        28310, 28349, 28388, 28427, 28467, 28506, 28545, 28584, 28624, 28663, 28702, 28742, 28781, 28821, 28860, 28900,
        28940, 28979, 29019, 29059, 29098, 29138, 29178, 29218, 29258, 29298, 29338, 29378, 29418, 29458, 29498, 29538,
        29579, 29619, 29659, 29699, 29740, 29780, 29821, 29861, 29902, 29942, 29983, 30023, 30064, 30105, 30146, 30186,
        30227, 30268, 30309, 30350, 30391, 30432, 30473, 30514, 30555, 30596, 30637, 30679, 30720, 30761, 30803, 30844,
        30885, 30927, 30968, 31010, 31052, 31093, 31135, 31177, 31218, 31257, 31296, 31336, 31375, 31415, 31455, 31494,
        31534, 31573, 31613, 31653, 31693, 31733, 31772, 31812, 31852, 31892, 31932, 31972, 32012, 32053, 32093, 32133,
        32173, 32214, 32254, 32294, 32335, 32375, 32416, 32456, 32497, 32537, 32578, 32619, 32659, 32700, 32741, 32782,
        32823, 32864, 32905, 32946, 32987, 33028, 33069, 33110, 33151, 33192, 33234, 33275, 33316, 33358, 33399, 33441,
        33482, 33524, 33565, 33607, 33649, 33690, 33732, 33774, 33816, 33858, 33900, 33942, 33984, 34026, 34068, 34110,
        34152, 34194, 34236, 34279, 34321, 34362, 34403, 34444, 34485, 34526, 34568, 34609, 34650, 34691, 34733, 34774,
        34815, 34857, 34898, 34940, 34982, 35023, 35065, 35106, 35148, 35190, 35232, 35274, 35316, 35358, 35400, 35442,
        35484, 35526, 35568, 35610, 35652, 35695, 35737, 35779, 35822, 35864, 35907, 35949, 35992, 36034, 36077, 36120,
        36162, 36205, 36248, 36291, 36334, 36377, 36420, 36463, 36506, 36549, 36592, 36636, 36679, 36722, 36765, 36809,
        36852, 36896, 36939, 36983, 37027, 37070, 37114, 37158, 37201, 37245, 37289, 37333, 37377, 37421, 37465, 37507,
        37550, 37593, 37636, 37678, 37721, 37764, 37807, 37850, 37893, 37936, 37980, 38023, 38066, 38109, 38153, 38196,
        38240, 38283, 38327, 38370, 38414, 38457, 38501, 38545, 38589, 38632, 38676, 38720, 38764, 38808, 38852, 38896,
        38941, 38985, 39029, 39073, 39118, 39162, 39206, 39251, 39295, 39340, 39385, 39429, 39474, 39519, 39564, 39608,
        39653, 39698, 39743, 39788, 39833, 39878, 39924, 39969, 40014, 40059, 40105, 40150, 40196, 40241, 40287, 40332,
        40378, 40424, 40470, 40515, 40561, 40605, 40649, 40692, 40736, 40780, 40824, 40867, 40911, 40955, 40999, 41043,
        41087, 41131, 41175, 41220, 41264, 41308, 41353, 41397, 41441, 41486, 41530, 41575, 41620, 41664, 41709, 41754,
        41799, 41844, 41888, 41933, 41978, 42023, 42069, 42114, 42159, 42204, 42249, 42295, 42340, 42386, 42431, 42477,
        42522, 42568, 42614, 42659, 42705, 42751, 42797, 42843, 42889, 42935, 42981, 43027, 43073, 43120, 43166, 43212,
        43259, 43305, 43352, 43398, 43445, 43491, 43538, 43585, 43632, 43679, 43724, 43770, 43816, 43861, 43907, 43953,
        43998, 44044, 44090, 44136, 44182, 44228, 44274, 44321, 44367, 44413, 44459, 44506, 44552, 44599, 44645, 44692,
        44738, 44785, 44832, 44879, 44925, 44972, 45019, 45066, 45113, 45160, 45208, 45255, 45302, 45349, 45397, 45444,
        45492, 45539, 45587, 45634, 45682, 45730, 45777, 45825, 45873, 45921, 45969, 46017, 46065, 46113, 46162, 46210,
        46258, 46307, 46355, 46404, 46452, 46501, 46549, 46598, 46647, 46696, 46745, 46794, 46841, 46888, 46936, 46983,
        47030, 47078, 47125, 47173, 47220, 47268, 47315, 47363, 47411, 47459, 47507, 47555, 47603, 47651, 47699, 47747,
        47795, 47843, 47892, 47940, 47989, 48037, 48086, 48134, 48183, 48232, 48280, 48329, 48378, 48427, 48476, 48525,
        48574, 48623, 48672, 48722, 48771, 48821, 48870, 48919, 48969, 49019, 49068, 49118, 49168, 49218, 49268, 49318,
        49368, 49418, 49468, 49518, 49568, 49619, 49669, 49720, 49770, 49821, 49871, 49922, 49971, 50020, 50069, 50118,
        50167, 50216, 50266, 50315, 50364, 50414, 50463, 50513, 50562, 50612, 50662, 50711, 50761, 50811, 50861, 50911,
        50961, 51011, 51061, 51112, 51162, 51212, 51263, 51313, 51364, 51414, 51465, 51516, 51566, 51617, 51668, 51719,
        51770, 51821, 51872, 51924, 51975, 52026, 52078, 52129, 52181, 52232, 52284, 52336, 52387, 52439, 52491, 52543,
        52595, 52647, 52699, 52751, 52804, 52856, 52908, 52961, 53013, 53066, 53118, 53170, 53223, 53275, 53328, 53380,
        53433, 53485, 53538, 53591, 53644, 53696, 53749, 53802, 53855, 53909, 53962, 54015, 54068, 54122, 54175, 54229,
        54283, 54336, 54390, 54444, 54498, 54552, 54606, 54660, 54714, 54768, 54822, 54877, 54931, 54986, 55040, 55095,
        55150, 55204, 55259, 55314, 55369, 55424, 55479, 55534, 55590, 55645, 55700, 55756, 55811, 55867, 55923, 55978,
        56034, 56090, 56146, 56202, 56258, 56314, 56370, 56427, 56483, 56540, 56596, 56653, 56709, 56766, 56823, 56880,
        56937, 56994, 57051, 57108, 57165, 57223, 57280, 57338, 57395, 57453, 57510, 57568, 57626, 57684, 57742, 57802,
        57863, 57923, 57984, 58044, 58105, 58166, 58226, 58287, 58348, 58409, 58470, 58532, 58593, 58654, 58716, 58777,
        58839, 58901, 58962, 59024, 59086, 59148, 59210, 59272, 59337, 59402, 59468, 59533, 59599, 59665, 59730, 59796,
        59862, 59928, 59995, 60061, 60127, 60194, 60260, 60327, 60394, 60461, 60527, 60594, 60661, 60729, 60796, 60863,
        60931, 60998, 61066, 61134, 61201, 61269, 61337, 61405, 61473, 61542, 61610, 61678, 61747, 61816, 61884, 61953,
        62022, 62091, 62160, 62229, 62299, 62368, 62441, 62522, 62603, 62684, 62765, 62846, 62927, 63009, 63090, 63172,
        63254, 63335, 63417, 63499, 63582, 63664, 63746, 63829, 63912, 63994, 64077, 64160, 64243, 64326, 64410, 64493,
        64577, 64660, 64744, 64828, 64912, 64996, 65080, 65165, 65249, 65334, 65419, 65503
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void crc_init(void) {
    RCC->AHBENR |= RCC_AHBENR_CRCEN;  // enable clock for CRC
    CRC->CR |= CRC_CR_RESET;   // Reset calculation
}

void transmit_serial() {
    crc_init();
    uint8_t i;
    CRC->CR = 1;
    for (i = 0; i < (154 + 8); i++) {
        CRC->DR = serial_out[i];
    }
    uint32_t crc[1] = {CRC->DR};
    memcpy(serial_out + 8 + 154, crc, 4);
    if (!bms_update) memset(serial_out + 8, 0x00, 154);
    HAL_UART_Transmit(&huart3, serial_out, 154 + 8 + 4, 100);
    bms_update = 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1) {
        memcpy(serial_out + 8, bms_rx_buffer, 154);
        bms_update = 1;
    } else if (huart == &huart3) {
        HAL_NVIC_SystemReset();
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    //1864
    //1863 1860 1873
    if (temp_in[0] > MAX) temp_out[0] = 0xFFFF;
    else if (temp_in[0] < MIN) temp_out[0] = 0x0000;
    else temp_out[0] =  TEMP_TABLE[temp_in[0] - MIN];

    if (temp_in[1] > MAX) temp_out[1] = 0xFFFF;
    else if (temp_in[1] < MIN) temp_out[1] = 0x0000;
    else temp_out[1] =  TEMP_TABLE[temp_in[1] - MIN];

    if (temp_in[2] > MAX) temp_out[2] = 0xFFFF;
    else if (temp_in[2] < MIN) temp_out[2] = 0x0000;
    else temp_out[2] =  TEMP_TABLE[temp_in[2] - MIN];

    if (temp_in[3] > MAX) temp_out[3] = 0xFFFF;
    else if (temp_in[3] < MIN) temp_out[3] = 0x0000;
    else temp_out[3] =  TEMP_TABLE[temp_in[3] - MIN];
    /*float temp_out[4];
    temp_out[0] =  (float)TEMP_TABLE[temp_in[0] - MIN] / (65535.0f / 210.0f) - 60;
    temp_out[1] =  (float)TEMP_TABLE[temp_in[1] - MIN] / (65535.0f / 210.0f) - 60;
    temp_out[2] =  (float)TEMP_TABLE[temp_in[2] - MIN] / (65535.0f / 210.0f) - 60;
    temp_out[3] =  (float)TEMP_TABLE[temp_in[3] - MIN] / (65535.0f / 210.0f) - 60;*/


    uint8_t c[70];
    int l = sprintf(c, "%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n", ((uint16_t)serial_out[1] << 8) | serial_out[0],
                    ((uint16_t)serial_out[3] << 8) | serial_out[2],
                    ((uint16_t)serial_out[5] << 8) | serial_out[4],
                    ((uint16_t)serial_out[7] << 8) | serial_out[6],
                    (serial_out[9] << 8) , serial_out[8],
                    (serial_out[11] << 8) , serial_out[10],
                    (serial_out[13] << 8) , serial_out[12],
                    (serial_out[15]) , serial_out[14],
                    serial_out[16]);
    CDC_Transmit_FS(c, l);


    memcpy(serial_out, temp_out, 8);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    transmit_serial();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) temp_in, 4);
    HAL_UART_Transmit(&huart1, BMS_REQUEST_DATA, 10, 100);
    HAL_UART_Receive_DMA(&huart1, bms_rx_buffer, 154);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();
    MX_USART3_UART_Init();
    MX_USB_DEVICE_Init();
    MX_TIM2_Init();
    MX_CRC_Init();
    /* USER CODE BEGIN 2 */
    bms_update = 0;
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_UART_Receive_DMA(&huart3, NULL, 1);


    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
