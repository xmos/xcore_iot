/* This example code is in the Public Domain (or CC0 licensed, at your option.)

   NO WARRANTY OR SUPPORT. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT
   WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL XMOS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN CONTRACT, WARRANTY, CIVIL TORT (INCLUDING
   NEGLIGENCE), PRODUCTS LIABILITY OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
   INCLUDING GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES EVEN IF SUCH
   PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES AND NOT
   WITHSTANDING THE FAILURE OF ESSENTIAL PURPOSE. IN SOME JURISDICTIONS PARTIES
   ARE UNABLE TO LIMIT LIABILTY IN THIS WAY, IF THIS APPLIES TO YOUR
   JURISDICTION THIS LIABILITY CLAUSE ABOVE MAY NOT APPLY. NOTWITHSTANDING THE
   ABOVE, IN NO EVENT SHALL XMOS’s TOTAL LIABILITY TO YOU FOR ALL DAMAGES, LOSS
   OR OTHERWISE EXCEED $50.
*/
#include <platform.h>
#include <xcore/chanend.h>

#include "FreeRTOS.h"
#include "task.h"

#include "rtos_printf.h"

#include "platform_init.h"

chanend_t other_tile_c;

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
  for (;;)
    ;
}

void startup_task(void *arg) {
  rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE,
              portGET_CORE_ID());

  for (;;) {
    rtos_printf("Hello from tile %d\n", THIS_XCORE_TILE);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // the forever loop above means we never reach here
  // if we did, we would want to free the resources
  chanend_free(other_tile_c);
  vTaskDelete(NULL);
}

static void tile_common_init(chanend_t c) {
  platform_init(c);

  xTaskCreate((TaskFunction_t)startup_task, "startup_task",
              RTOS_THREAD_STACK_SIZE(startup_task), NULL,
              configMAX_PRIORITIES - 1, NULL);

  rtos_printf("Start scheduler on tile %d\n", THIS_XCORE_TILE);
  vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {

  (void)c0;
  (void)c2;
  (void)c3;

  tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
  (void)c1;
  (void)c2;
  (void)c3;

  tile_common_init(c0);
}
#endif