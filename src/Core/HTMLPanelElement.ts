import { Panel } from './Panel';
import { UIControl } from './UIControls';

export class HTMLPanelElement extends Panel {
    protected element: HTMLElement | null = null;

    constructor(name: string, element?: HTMLElement) {
        super(name);
        if (element) {
            this.setElement(element);
        }
    }

    public setElement(element: HTMLElement): this {
        this.element = element;
        return this;
    }

    public getElement(): HTMLElement | null {
        return this.element;
    }

    public addChild(child: Panel | UIControl | HTMLElement): void {
        if (child instanceof HTMLElement) {
            const panel = new HTMLPanelElement(child.id || 'element', child);
            super.addChild(panel);
        } else {
            super.addChild(child as Panel);
        }
    }

    public refresh(): void {
        super.refresh();
        if (this.element) {
            const computedStyle = this.getStyle().getComputedStyle();
            Object.assign(this.element.style, {
                backgroundColor: computedStyle.backgroundColor,
                color: computedStyle.foregroundColor,
                borderColor: computedStyle.borderColor,
                borderWidth: `${computedStyle.borderWidth}px`,
                borderRadius: `${computedStyle.borderRadius}px`,
                borderStyle: computedStyle.borderStyle,
                padding: `${computedStyle.padding}px`,
                opacity: computedStyle.opacity,
                position: 'absolute',
                left: `${this.getPosition().x}px`,
                top: `${this.getPosition().y}px`,
                width: `${this.getSize().width}px`,
                height: `${this.getSize().height}px`
            });
        }
    }
}
