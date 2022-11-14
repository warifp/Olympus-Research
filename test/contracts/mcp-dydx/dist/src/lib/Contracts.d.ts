import { Provider } from 'web3/providers';
import Web3 from 'web3';
import { TransactionObject } from 'web3/eth/types';
import { SoloMargin } from '../../build/wrappers/SoloMargin';
import { IErc20 as ERC20 } from '../../build/wrappers/IErc20';
import { IInterestSetter as InterestSetter } from '../../build/wrappers/IInterestSetter';
import { IPriceOracle as PriceOracle } from '../../build/wrappers/IPriceOracle';
import { Expiry } from '../../build/wrappers/Expiry';
import { ExpiryV2 } from '../../build/wrappers/ExpiryV2';
import { FinalSettlement } from '../../build/wrappers/FinalSettlement';
import { Refunder } from '../../build/wrappers/Refunder';
import { DaiMigrator } from '../../build/wrappers/DaiMigrator';
import { LimitOrders } from '../../build/wrappers/LimitOrders';
import { StopLimitOrders } from '../../build/wrappers/StopLimitOrders';
import { CanonicalOrders } from '../../build/wrappers/CanonicalOrders';
import { PayableProxyForSoloMargin as PayableProxy } from '../../build/wrappers/PayableProxyForSoloMargin';
import { SignedOperationProxy } from '../../build/wrappers/SignedOperationProxy';
import { LiquidatorProxyV1ForSoloMargin as LiquidatorProxyV1 } from '../../build/wrappers/LiquidatorProxyV1ForSoloMargin';
import { PolynomialInterestSetter } from '../../build/wrappers/PolynomialInterestSetter';
import { DoubleExponentInterestSetter } from '../../build/wrappers/DoubleExponentInterestSetter';
import { WethPriceOracle } from '../../build/wrappers/WethPriceOracle';
import { DaiPriceOracle } from '../../build/wrappers/DaiPriceOracle';
import { UsdcPriceOracle } from '../../build/wrappers/UsdcPriceOracle';
import { WETH9 } from '../../build/wrappers/WETH9';
import { TxResult, address, SoloOptions, CallOptions, SendOptions } from '../types';
export declare class Contracts {
    private blockGasLimit;
    private autoGasMultiplier;
    private defaultConfirmations;
    private confirmationType;
    private defaultGas;
    private defaultGasPrice;
    protected web3: Web3;
    soloMargin: SoloMargin;
    erc20: ERC20;
    interestSetter: InterestSetter;
    priceOracle: PriceOracle;
    expiry: Expiry;
    expiryV2: ExpiryV2;
    finalSettlement: FinalSettlement;
    refunder: Refunder;
    daiMigrator: DaiMigrator;
    limitOrders: LimitOrders;
    stopLimitOrders: StopLimitOrders;
    canonicalOrders: CanonicalOrders;
    payableProxy: PayableProxy;
    signedOperationProxy: SignedOperationProxy;
    liquidatorProxyV1: LiquidatorProxyV1;
    polynomialInterestSetter: PolynomialInterestSetter;
    doubleExponentInterestSetter: DoubleExponentInterestSetter;
    wethPriceOracle: WethPriceOracle;
    daiPriceOracle: DaiPriceOracle;
    saiPriceOracle: DaiPriceOracle;
    usdcPriceOracle: UsdcPriceOracle;
    weth: WETH9;
    constructor(provider: Provider, networkId: number, web3: Web3, options: SoloOptions);
    setProvider(provider: Provider, networkId: number): void;
    setDefaultAccount(account: address): void;
    send<T>(method: TransactionObject<T>, options?: SendOptions): Promise<TxResult>;
    call<T>(method: TransactionObject<T>, options?: CallOptions): Promise<T>;
    private setGasLimit;
    protected setContractProvider(contract: any, contractJson: any, provider: Provider, networkId: number, overrides: any): void;
    private toEstimateOptions;
    private toCallOptions;
    private toNativeSendOptions;
    private normalizeResponse;
}
