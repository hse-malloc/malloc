#include <unistd.h>
#include <malloc.hpp>

namespace  {

static int has_initialized = 0;
static void *managed_memory_start;
static void *last_valid_address;

void malloc_init()
{
    /* захватить (запросить у системы) последний валидный адрес */
    last_valid_address = sbrk(0);

    /* пока у нас нет памяти, которой можно было бы управлять
     * установим начальный указатель на last_valid_address
     */
    managed_memory_start = last_valid_address;

    /* Инициализация прошла, теперь можно пользоваться */
    has_initialized = 1;
}


void mem_free(void *firstbyte) {
    mem_control_block *mcb;

    /* Отматываем текущий указатель и работаем с ним как с
     * mem_control_block
     */
    mcb = reinterpret_cast<mem_control_block*>(reinterpret_cast<char *>(firstbyte) - sizeof(mem_control_block));

    /* Помечаем блок как доступный */
    mcb->is_available = 1;

    /* Всё готово! */
    return;
}


void * mem_alloc(long numbytes) {
    /* Место откуда начинается поиск */
    void *current_location;

    /* Представим что мы работаем с
     * memory_control_block
     */
    mem_control_block *current_location_mcb;

    /* В этот указатель мы вернём найденную память. На время поиска он должен быть 0 */
    void *memory_location = nullptr;

    /* Инициализируем, если мы этого не сделали */
    if(! has_initialized) 	{
        malloc_init();
    }

    /* Память содержит в себе memory
     * control block, но пользователям функции mallocне нужно
     * об этом знать. Просто смещаем указатель на размер структуры
     */
    numbytes = numbytes + sizeof(mem_control_block);

    /* Присваиваем memory_location 0 пока не найдем подходящий участок */
    memory_location = 0;

    /* Начинаем поиск с начала доступной (управляемой) памяти */
    current_location = managed_memory_start;

    /* Ищем по всему доступному пространству  */
    while(current_location != last_valid_address)
    {
        /* По факту current_location и current_location_mcb
         * одинаковые адреса.  Но current_location_mcb
         * мы используем как структуру , а
         * current_location как указатель для перемещенияt
         */
        current_location_mcb =
            (struct mem_control_block *)current_location;

        if(current_location_mcb->is_available)
        {
            if(current_location_mcb->size >= numbytes)
            {
                /* Воооу! Мы нашли подходящий блок... */

                /* Кто первым встал, того и тапки - отмечаем участок как занятый */
                current_location_mcb->is_available = 0;

                /* Мы оккупировали эту территорию */
                memory_location = current_location;

                /* Прекращаем цикл */
                break;
            }
        }

        /* Если мы оказались здесь, это потому что текущиё блок памяти нам не подошёл, сяпаем дальше */
        current_location = reinterpret_cast<char *>(current_location) +
            current_location_mcb->size;
    }

    /* Если мы всё ещё не имеем подходящего адреса, то следует запросить память у ОС */
    if(! memory_location)
    {
        /* Move the program break numbytes further */
        sbrk(numbytes);

        /* После выделения, last_valid_address должен обновится */
        memory_location = last_valid_address;

        /* Перемещаемся от last valid address на
         * numbytes вперёд
         */
        last_valid_address = reinterpret_cast<char *>(last_valid_address) + numbytes;

        /* И инициализируем mem_control_block */
        current_location_mcb = reinterpret_cast<mem_control_block *>(memory_location);
        current_location_mcb->is_available = 0;
        current_location_mcb->size = numbytes;

    }

    /* Теперь мы получили память (если не получили ошибок).
     * И в memory_location также есть место под
     * mem_control_block
     */

    /* Перемещаем указатель в конец mem_control_block */
    memory_location = reinterpret_cast<char* >(memory_location) + sizeof(struct mem_control_block);

    /* Возвращаем указатель */
    return memory_location;

 }

}
