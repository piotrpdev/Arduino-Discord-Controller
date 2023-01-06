/**
 * @name VoiceToWS
 * @author Piotr Placzek
 * @description Streams Voice Channel data to WebSocket
 * @version 0.8.1
 */

module.exports = class SerialFlux {
    start() {
        // ? https://rauenzi.github.io/BDPluginLibrary/docs/modules_discordmodules.js.html
        this.FluxDispatcher = BdApi.Webpack.getModule(BdApi.Webpack.Filters.byProps('dispatch', 'register'));
        this.UserStore = BdApi.Webpack.getModule(BdApi.Webpack.Filters.byProps('getCurrentUser'));
        this.VoiceStore = BdApi.Webpack.getModule(BdApi.Webpack.Filters.byProps("getNoiseCancellation"));
        this.VoiceSetStore = BdApi.Webpack.getModule(BdApi.Webpack.Filters.byProps("setNoiseCancellation"));
        this.ChannelActions = BdApi.Webpack.getModule(BdApi.Webpack.Filters.byProps("selectChannel"));

        this.log('Opening WebSocket');
        this.ws = new WebSocket('ws://127.0.0.1:8000');

        this.wsOnOpen = this._wsOnOpen.bind(this);
        this.wsSend = this._wsSend.bind(this);
        this.wsOnMessage = this._wsOnMessage.bind(this);

        this.ws.addEventListener('open', this.wsOnOpen);
        this.ws.addEventListener('message', this.wsOnMessage);
        // document.addEventListener('keydown', (e) => {
        //     if (e.key === 'k') {
        //         this.wsSend({ type: 'KEYDOWN', key: 'k' });
        //     }
        // });
    }

    _wsOnOpen() {
        this.log(`Connected to ${this.ws.url}`);

        this.userId = this.UserStore.getCurrentUser().id;
        const localVoiceVars = this.VoiceStore.__getLocalVars();
        const defaultVoice = localVoiceVars.settingsByContext.default

        const pick = (obj, keys) => Object.fromEntries(Object.entries(obj).filter(([key]) => keys.includes(key)));

        const initialVoiceSettings = pick(localVoiceVars, ['currentVoiceChannelId', 'serverDeaf', 'serverMute'])
        initialVoiceSettings.local = pick(defaultVoice, ['mute', 'deaf'])
        
        this.wsSend({ type: 'SYN', userId: this.userId, initialVoiceSettings });

        //this.FluxDispatcher.subscribe('VOICE_STATE_UPDATES', this.wsSend);
        this.FluxDispatcher.subscribe('RTC_CONNECTION_STATE', this.wsSend);
        this.FluxDispatcher.subscribe('AUDIO_TOGGLE_SELF_MUTE', this.wsSend);
        this.FluxDispatcher.subscribe('AUDIO_TOGGLE_SELF_DEAF', this.wsSend);
    }

    _wsSend(msg) {
        if (this.ws.readyState === this.ws.OPEN) {
            // dont clog the websocket with other users voice state updates
            if (msg.type == 'VOICE_STATE_UPDATES' && !msg.voiceStates.find(v => v.userId == this.userId)) return;
            // only care if user fully connected or disconnected
            if (msg.type == 'RTC_CONNECTION_STATE' && !(msg.state == "RTC_CONNECTED" || msg.state == "DISCONNECTED")) return;
            if ((msg.type == 'AUDIO_TOGGLE_SELF_MUTE' || msg.type == 'AUDIO_TOGGLE_SELF_DEAF')) {
                // dont send if in vc, 'VOICE_STATE_UPDATES' will handle it
                // if (this.VoiceStore.getMediaEngine().connections.size !== 0) return;
                msg = { type: 'AUDIO_TOGGLE', isMute: this.VoiceStore.isSelfMute(), isDeaf: this.VoiceStore.isSelfDeaf()};
            }
            this.log("Sending", msg);
            this.ws.send(JSON.stringify(msg))
        } else {
            this.log('WebSocket is not open, try restarting plugin');
        }
    }

    _wsOnMessage({ data: msg }) {
        let parsedMsg = JSON.parse(msg);
        this.log('Received', parsedMsg);

        if (parsedMsg.type == "SERIAL") {
            const serialData = parsedMsg.data.slice(1, -1);

            if (serialData.charAt(0) === '#') {
                switch (serialData.charAt(1)) {
                    case 'M':
                        if (serialData.charAt(2) == !this.VoiceStore.isSelfMute()) return;
                        this.VoiceSetStore.toggleSelfMute();
                        break;
                    case 'H':
                        if (serialData.charAt(2) == !this.VoiceStore.isSelfDeaf()) return;
                        this.VoiceSetStore.toggleSelfDeaf();
                        break;
                    case 'C':
                        let media = this.VoiceStore.getMediaEngine();
                        if (media.connections.size == 0) return;
                        this.ChannelActions.disconnect();
                        break;
                    default:
                        break;
                }
            }
        }
    }

    log(...args) {
        console.log("%c[VoiceToWS]", "color: yellow", ...args);
    }

    stop() {
        //this.FluxDispatcher.unsubscribe('VOICE_STATE_UPDATES', this.wsSend);
        this.FluxDispatcher.unsubscribe('RTC_CONNECTION_STATE', this.wsSend);
        this.FluxDispatcher.unsubscribe('AUDIO_TOGGLE_SELF_MUTE', this.wsSend);
        this.FluxDispatcher.unsubscribe('AUDIO_TOGGLE_SELF_DEAF', this.wsSend);
        this.ws.removeEventListener('open', this.wsOnOpen);
        this.ws.removeEventListener('message', this.wsOnMessage);

        this.log('Closing WebSocket');
        this.ws.close();
    }
}