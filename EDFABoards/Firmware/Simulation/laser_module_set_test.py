import asyncio
import websockets

async def listen_for_message(websocket):

    while True:

        await asyncio.sleep(0)

        try:
            print('Listening for a message...')
            message = await websocket.recv()

            print("< {}".format(message))

        except websockets.ConnectionClosed as cc:
            print('Connection closed')

        except Exception as e:
            print('Something happened')


async def connect_to_dealer():

    print('connect to dealer')
    websocket = await websockets.connect('ws://localhost:8000/set_laser_module_2')
    return websocket


async def my_app():

    # this will block until connect_to_dealer() returns
    websocket = await connect_to_dealer()

    # start listen_for_message() in its own task wrapper, so doing it continues in the background
    asyncio.ensure_future(listen_for_message(websocket))

    # you can continue with other code here that can now coexist with listen_for_message()


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(my_app())
    loop.run_forever() 